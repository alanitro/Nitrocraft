#include "World_ChunkManager.hpp"

#include <algorithm>
#include "World_Generation.hpp"
#include "World_Light.hpp"
#include "World_Mesh.hpp"

World_ChunkManager::World_ChunkManager()
{
    m_ChunkMap.reserve(World_LOADING_AREA * 2);

    m_WorkerCount = std::clamp(std::thread::hardware_concurrency(), 1u, 16u);

    m_Workers.reserve(m_WorkerCount);
    
    for (std::size_t i = 0u; i < m_WorkerCount; ++i)
    {
        m_Workers.emplace_back([this] { World_Generation_Initialize(12345); JobLoop(); });
    }
}

World_ChunkManager::~World_ChunkManager()
{
    m_JobRetire.store(true, std::memory_order_release);

    m_JobQueueCond.notify_all();

    m_Workers.clear();
}

void World_ChunkManager::SetCenterChunkMainThread(World_ChunkID center_id)
{
    if (m_CurrentCenterID == center_id) return;

    m_CurrentCenterID = center_id;

    // Update chunk map.

    // Loaded area's outermost ring's chunks are NOT neighbour set.
    World_Chunk* loading_area[World_LOADING_DIAMETER][World_LOADING_DIAMETER];

    // Chunks that are not in loaded area are allocated in ChunkMap.
    {
        std::lock_guard<std::mutex> lock{ m_ChunkMapMutex };

        for (int ix = m_CurrentCenterID.x - World_LOADING_RADIUS, ax = 0; ix <= m_CurrentCenterID.x + World_LOADING_RADIUS; ++ix, ++ax)
        {
            for (int iz = m_CurrentCenterID.z - World_LOADING_RADIUS, az = 0; iz <= m_CurrentCenterID.z + World_LOADING_RADIUS; ++iz, ++az)
            {
                World_ChunkID id{ ix, 0, iz };

                if (auto iter = m_ChunkMap.find(id); iter == m_ChunkMap.end())
                {
                    auto new_chunk = std::make_unique<World_Chunk>(id);

                    new_chunk->Storage = std::make_unique<World_Chunk_Storage>();

                    loading_area[ax][az] = new_chunk.get();

                    m_ChunkMap.emplace(id, std::move(new_chunk));
                }
                else
                {
                    loading_area[ax][az] = iter->second.get();
                }
            }
        }
    }

    // Associate neighbours for non-outermost ring chunks.
    for (int i = 1; i < World_LOADING_DIAMETER - 1; ++i)
    {
        for (int j = 1; j < World_LOADING_DIAMETER - 1; ++j)
        {
            auto c = loading_area[i][j];

            if (c->NeighboursSet) continue;

            c->Neighbours[(std::size_t)World_Chunk_Neighbour::XNZ0] = loading_area[i - 1][j];
            c->Neighbours[(std::size_t)World_Chunk_Neighbour::XPZ0] = loading_area[i + 1][j];
            c->Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZN] = loading_area[i][j - 1];
            c->Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZP] = loading_area[i][j + 1];
            c->Neighbours[(std::size_t)World_Chunk_Neighbour::XNZN] = loading_area[i - 1][j - 1];
            c->Neighbours[(std::size_t)World_Chunk_Neighbour::XPZN] = loading_area[i + 1][j - 1];
            c->Neighbours[(std::size_t)World_Chunk_Neighbour::XNZP] = loading_area[i - 1][j + 1];
            c->Neighbours[(std::size_t)World_Chunk_Neighbour::XPZP] = loading_area[i + 1][j + 1];

            c->NeighboursSet = true;
        }
    }

    // Schedule work for render area.
    // Render area is an area within ring3 excluding three-outermost-ring area.
    {
        std::lock_guard<std::mutex> lock{ m_JobQueueMutex };

        for (int i = 3; i < World_LOADING_DIAMETER - 3; ++i)
        for (int j = 3; j < World_LOADING_DIAMETER - 3; ++j)
        {
            World_Chunk* c = loading_area[i][j];
            if (c->Stage.load(std::memory_order_acquire) < World_Chunk_Stage::MeshingInProgress)
                EnqueueDedupJob_ThreadUnsafe({ c, JobType::Meshing });
        }
    }

    m_JobQueueCond.notify_one();
}

std::vector<World_Chunk*> World_ChunkManager::GetChunksInRenderArea() const
{
    std::vector<World_Chunk*> chunks_to_render;

    {
        std::lock_guard<std::mutex> lock{ m_ChunkMapMutex };

        for (int ix = m_CurrentCenterID.x - World_LOADING_RADIUS + 3; ix <= m_CurrentCenterID.x + World_LOADING_RADIUS - 3; ++ix)
        for (int iz = m_CurrentCenterID.z - World_LOADING_RADIUS + 3; iz <= m_CurrentCenterID.z + World_LOADING_RADIUS - 3; ++iz)
        {
            if (auto iter = m_ChunkMap.find(World_ChunkID(ix, 0, iz)); iter != m_ChunkMap.end())
            {
                chunks_to_render.push_back(iter->second.get());
            }
        }
    }

    return chunks_to_render;
}

std::optional<const World_Chunk*> World_ChunkManager::GetChunkAt(World_GlobalXYZ global) const
{
    std::lock_guard<std::mutex> lock{ m_ChunkMapMutex };

    if (auto iter = m_ChunkMap.find(World_FromGlobalToChunkID(global)); iter != m_ChunkMap.end())
    {
        return iter->second.get();
    }
    else
    {
        return std::nullopt;
    }
}

void World_ChunkManager::JobLoop()
{
    while (true)
    {
        Job job;

        {
            std::unique_lock lock{ m_JobQueueMutex };

            m_JobQueueCond.wait(lock, [this]() { return m_JobRetire.load(std::memory_order_acquire) || !m_JobQueue.empty(); });

            if (m_JobRetire.load(std::memory_order_acquire)) return;

            job = m_JobQueue.front(); m_JobQueue.pop();

            job.Chunk->EnqueuedStates.fetch_and(static_cast<std::uint8_t>(~JobBit(job.Type)), std::memory_order_acq_rel);
        }

        if (job.Type == JobType::Generation)
        {
            GenerationJobHandler(job.Chunk);
        }
        else if (job.Type == JobType::LocalLighting)
        {
            LocalLightingJobHandler(job.Chunk);
        }
        else if (job.Type == JobType::NeighbourLighting)
        {
            NeighbourLightingJobHandler(job.Chunk);
        }
        else if (job.Type == JobType::Meshing)
        {
            MeshingJobHandler(job.Chunk);
        }
    }
}

void World_ChunkManager::EnqueueDedupJob_ThreadSafeWithNotify(Job job)
{
    if (job.Chunk->EnqueuedStates.fetch_or(JobBit(job.Type), std::memory_order_relaxed) & JobBit(job.Type)) return;

    {
        std::lock_guard<std::mutex> lock{ m_JobQueueMutex };

        m_JobQueue.push(job);
    }

    m_JobQueueCond.notify_one();
}

void World_ChunkManager::EnqueueDedupJob_ThreadUnsafe(Job job)
{
    if (job.Chunk->EnqueuedStates.fetch_or(JobBit(job.Type), std::memory_order_relaxed) & JobBit(job.Type)) return;

    m_JobQueue.push(job);
}

void World_ChunkManager::GenerationJobHandler(World_Chunk* chunk)
{
    // Called chunk is in Stage==Empty -> ready for terrain/cave generation.
    auto expected = World_Chunk_Stage::Empty;
    if (!chunk->Stage.compare_exchange_strong(expected, World_Chunk_Stage::GenerationInProgress, std::memory_order_acq_rel, std::memory_order_acquire)) return;

    World_Generation_GenerateChunk(chunk);

    chunk->Stage.store(World_Chunk_Stage::GenerationComplete, std::memory_order_release);
}

void World_ChunkManager::LocalLightingJobHandler(World_Chunk* chunk)
{
    // Called chunk has to be in Stage==GenerationComplete state 
    if (chunk->Stage.load(std::memory_order_acquire) < World_Chunk_Stage::GenerationComplete)
    {
        {
            std::lock_guard<std::mutex> lock{ m_JobQueueMutex };

            EnqueueDedupJob_ThreadUnsafe({ chunk, JobType::Generation });
            EnqueueDedupJob_ThreadUnsafe({ chunk, JobType::LocalLighting });
        }

        m_JobQueueCond.notify_one();

        return;
    }

    // Called chunk's neighbours have to be in Stage>=GenerationComplete state
    {
        std::array<World_Chunk*, static_cast<std::size_t>(World_Chunk_Neighbour::COUNT)> missings{};
        auto missings_iter = missings.begin();

        for (auto neighbour : chunk->Neighbours)
        {
            if (neighbour->Stage.load(std::memory_order_acquire) >= World_Chunk_Stage::GenerationComplete) continue;

            *missings_iter = neighbour;
            missings_iter++;
        }

        {
            std::lock_guard<std::mutex> lock{ m_JobQueueMutex };

            for (auto iter = missings.begin(); iter != missings_iter; ++iter)
            {
                EnqueueDedupJob_ThreadUnsafe(Job{ *iter, JobType::Generation });
            }

            if (missings_iter != missings.begin())
            {
                EnqueueDedupJob_ThreadUnsafe(Job{ chunk, JobType::LocalLighting });
            }
        }

        if (missings_iter != missings.begin())
        {
            m_JobQueueCond.notify_one();
            return;
        }
    }

    // Above conditionas are met -> start propagating local lights
    World_Chunk_Stage expected = World_Chunk_Stage::GenerationComplete;
    if (!chunk->Stage.compare_exchange_strong(expected, World_Chunk_Stage::LocalLightingInProgress, std::memory_order_acq_rel, std::memory_order_acquire)) return;

    World_Light_PropagateInitialSunlight(chunk);

    chunk->Stage.store(World_Chunk_Stage::LocalLightingComplete, std::memory_order_release);
}

void World_ChunkManager::NeighbourLightingJobHandler(World_Chunk* chunk)
{
    // Called chunk has to be in Stage==LocalLightingComplete.
    if (chunk->Stage.load(std::memory_order_acquire) < World_Chunk_Stage::LocalLightingComplete)
    {
        {
            std::lock_guard<std::mutex> lock{ m_JobQueueMutex };

            EnqueueDedupJob_ThreadUnsafe({ chunk, JobType::LocalLighting });
            EnqueueDedupJob_ThreadUnsafe({ chunk, JobType::NeighbourLighting});
        }

        m_JobQueueCond.notify_one();

        return;
    }

    // Called chunk's neighbours have to be in Stage>=LocalLightingComplete state.
    {
        std::array<World_Chunk*, static_cast<std::size_t>(World_Chunk_Neighbour::COUNT)> missings{};
        auto missings_iter = missings.begin();

        for (auto neighbour : chunk->Neighbours)
        {
            if (neighbour->Stage.load(std::memory_order_acquire) >= World_Chunk_Stage::LocalLightingComplete) continue;

            *missings_iter = neighbour;
            missings_iter++;
        }

        {
            std::lock_guard<std::mutex> lock{ m_JobQueueMutex };

            for (auto iter = missings.begin(); iter != missings_iter; ++iter)
            {
                EnqueueDedupJob_ThreadUnsafe(Job{ *iter, JobType::LocalLighting });
            }

            if (missings_iter != missings.begin())
            {
                EnqueueDedupJob_ThreadUnsafe(Job{ chunk, JobType::NeighbourLighting });
            }
        }

        if (missings_iter != missings.begin())
        {
            m_JobQueueCond.notify_one();
            return;
        }
    }

    // Above conditions are met -> called chunk's lighting is complete (for now).
    World_Chunk_Stage expected = World_Chunk_Stage::LocalLightingComplete;
    if (!chunk->Stage.compare_exchange_strong(expected, World_Chunk_Stage::NeighbourLightingInProgress, std::memory_order_acq_rel, std::memory_order_acquire)) return;

    // TODO: neighbour light propagation

    chunk->Stage.store(World_Chunk_Stage::NeighbourLightingComplete, std::memory_order_release);
}

void World_ChunkManager::MeshingJobHandler(World_Chunk* chunk)
{
    // Called chunk has to be in Stage==NeighbourLightingComplete.
    if (chunk->Stage.load(std::memory_order_acquire) < World_Chunk_Stage::NeighbourLightingComplete)
    {
        {
            std::lock_guard<std::mutex> lock{ m_JobQueueMutex };

            EnqueueDedupJob_ThreadUnsafe({ chunk, JobType::NeighbourLighting});
            EnqueueDedupJob_ThreadUnsafe({ chunk, JobType::Meshing});
        }

        m_JobQueueCond.notify_one();

        return;
    }

    // Above condition ensures that the called chunk is in meshable state.
    World_Chunk_Stage expected = World_Chunk_Stage::NeighbourLightingComplete;
    if (!chunk->Stage.compare_exchange_strong(expected, World_Chunk_Stage::MeshingInProgress, std::memory_order_acq_rel, std::memory_order_acquire)) return;

    chunk->CPUMesh.Vertices.clear();
    chunk->CPUMesh.Indices.clear();

    World_Mesh_GenerateChunkCPUMesh(chunk, chunk->CPUMesh);

    chunk->Stage.store(World_Chunk_Stage::MeshingComplete, std::memory_order_release);
}
