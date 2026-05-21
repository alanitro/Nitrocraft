#include "World_ChunkManager.hpp"

#include <algorithm>
#include "World_TerrainGenerator.hpp"
#include "World_Light.hpp"

void nitrocraft::world::ChunkManager::Initialize()
{
    m_chunk_map.reserve(GetLoadingDiameter() * GetLoadingDiameter() * 8);

    m_worker_count = std::clamp<std::size_t>(std::thread::hardware_concurrency() / 2 - 1, 1u, 4u);

    m_workers.reserve(m_worker_count);

    m_terrain_generator.Initialize();

    for (std::size_t i = 0u; i < m_worker_count; ++i)
    {
        m_workers.emplace_back([this] { JobLoop(); });
    }
}

void nitrocraft::world::ChunkManager::Terminate()
{
    m_job_retire.store(true, std::memory_order_release);

    m_job_queue_cond.notify_all();

    m_workers.clear();
}

void nitrocraft::world::ChunkManager::SetCenterChunk_MainThread(ChunkID center_id)
{
    static std::size_t prev_render_distance = m_render_distance;

    if (m_current_chunk_id == center_id && prev_render_distance == m_render_distance) return;

    m_current_chunk_id = center_id;
    prev_render_distance = m_render_distance;

    // Update chunk map.
    // Loaded area's outermost ring's chunks are NOT neighbour set.
    const int loading_distance = static_cast<int>(GetLoadingDistance());
    const int loading_diameter = static_cast<int>(GetLoadingDiameter());

    std::vector<Chunk*> loading_area(loading_diameter * loading_diameter, nullptr);

    auto index_of = [loading_diameter](int i, int j) { return i * loading_diameter + j; };

    // Chunks that are not in loaded area are allocated in ChunkMap.
    {
        std::lock_guard<std::mutex> lock{ m_chunk_map_mutex };

        for (int ix = m_current_chunk_id.x - loading_distance, ax = 0; ix <= m_current_chunk_id.x + loading_distance; ++ix, ++ax)
        for (int iz = m_current_chunk_id.z - loading_distance, az = 0; iz <= m_current_chunk_id.z + loading_distance; ++iz, ++az)
        {
            ChunkID id{ ix, 0, iz };

            auto& slot = loading_area[index_of(ax, az)];

            if (auto iter = m_chunk_map.find(id); iter == m_chunk_map.end())
            {
                auto new_chunk = std::make_unique<Chunk>(id);

                new_chunk->storage = std::make_unique<ChunkStorage>();

                slot = new_chunk.get();

                m_chunk_map.emplace(id, std::move(new_chunk));
            }
            else
            {
                slot = iter->second.get();
            }
        }
    }

    // Associate neighbours for non-outermost ring chunks.
    for (int i = 1; i < loading_diameter - 1; ++i)
    for (int j = 1; j < loading_diameter - 1; ++j)
    {
        auto c = loading_area[index_of(i,j)];

        if (c->neighbours_set.load(std::memory_order_relaxed)) continue;

        c->neighbours[(std::size_t)ChunkNeighbour::XNZ0] = loading_area[index_of(i - 1, j    )];
        c->neighbours[(std::size_t)ChunkNeighbour::XPZ0] = loading_area[index_of(i + 1, j    )];
        c->neighbours[(std::size_t)ChunkNeighbour::X0ZN] = loading_area[index_of(i    , j - 1)];
        c->neighbours[(std::size_t)ChunkNeighbour::X0ZP] = loading_area[index_of(i    , j + 1)];
        c->neighbours[(std::size_t)ChunkNeighbour::XNZN] = loading_area[index_of(i - 1, j - 1)];
        c->neighbours[(std::size_t)ChunkNeighbour::XPZN] = loading_area[index_of(i + 1, j - 1)];
        c->neighbours[(std::size_t)ChunkNeighbour::XNZP] = loading_area[index_of(i - 1, j + 1)];
        c->neighbours[(std::size_t)ChunkNeighbour::XPZP] = loading_area[index_of(i + 1, j + 1)];

        c->neighbours_set.store(true, std::memory_order_release);
    }

    // Schedule work for render area.
    // Render area is an area within ring3 excluding three-outermost-ring area.
    {
        std::lock_guard<std::mutex> lock{ m_job_queue_mutex };

        for (int i = 3; i < loading_diameter - 3; ++i)
        for (int j = 3; j < loading_diameter - 3; ++j)
        {
            Chunk* c = loading_area[index_of(i, j)];
            if (c->stage.load(std::memory_order_acquire) < ChunkStage::NeighbourLightingInProgress)
                EnqueueDedupJob_ThreadUnsafe({ c, JobType::NeighbourLighting });
        }
    }

    m_job_queue_cond.notify_one();
}

std::vector<nitrocraft::world::Chunk*> nitrocraft::world::ChunkManager::GetChunksInRenderArea_MainThread() const
{
    std::vector<Chunk*> chunks_to_render;

    {
        std::lock_guard<std::mutex> lock{ m_chunk_map_mutex };

        for (int ix = m_current_chunk_id.x - static_cast<int>(m_render_distance); ix <= m_current_chunk_id.x + static_cast<int>(m_render_distance); ++ix)
        for (int iz = m_current_chunk_id.z - static_cast<int>(m_render_distance); iz <= m_current_chunk_id.z + static_cast<int>(m_render_distance); ++iz)
        {
            if (auto iter = m_chunk_map.find(ChunkID(ix, 0, iz)); iter != m_chunk_map.end())
            {
                chunks_to_render.push_back(iter->second.get());
            }
        }
    }

    return chunks_to_render;
}

std::optional<const nitrocraft::world::Chunk*> nitrocraft::world::ChunkManager::GetChunkAt(GlobalXYZ global) const
{
    std::lock_guard<std::mutex> lock{ m_chunk_map_mutex };

    if (auto iter = m_chunk_map.find(FromGlobalToChunkID(global)); iter != m_chunk_map.end())
    {
        return iter->second.get();
    }
    else
    {
        return std::nullopt;
    }
}

void nitrocraft::world::ChunkManager::SetRenderDistance(std::size_t render_distance)
{
    m_render_distance = std::clamp<std::size_t>(render_distance, 2, 32);
}

std::size_t nitrocraft::world::ChunkManager::GetLoadingDistance() const
{
    return m_render_distance + 3;
}

std::size_t nitrocraft::world::ChunkManager::GetLoadingDiameter() const
{
    return GetLoadingDistance() * 2 + 1;
}

void nitrocraft::world::ChunkManager::JobLoop()
{
    while (true)
    {
        Job job;

        {
            std::unique_lock lock{ m_job_queue_mutex };

            m_job_queue_cond.wait(lock, [this]() { return m_job_retire.load(std::memory_order_acquire) || !m_job_queue.empty(); });

            if (m_job_retire.load(std::memory_order_acquire)) return;

            job = m_job_queue.front(); m_job_queue.pop();

            job.chunk->enqueued_states.fetch_and(static_cast<std::uint8_t>(~JobBit(job.type)), std::memory_order_acq_rel);
        }

        if (job.type == JobType::Generation)
        {
            GenerationJobHandler(job.chunk);
        }
        else if (job.type == JobType::LocalLighting)
        {
            LocalLightingJobHandler(job.chunk);
        }
        else if (job.type == JobType::NeighbourLighting)
        {
            NeighbourLightingJobHandler(job.chunk);
        }
    }
}

void nitrocraft::world::ChunkManager::EnqueueDedupJob_ThreadSafeWithNotify(Job job)
{
    if (job.chunk->enqueued_states.fetch_or(JobBit(job.type), std::memory_order_relaxed) & JobBit(job.type)) return;

    {
        std::lock_guard<std::mutex> lock{ m_job_queue_mutex };

        m_job_queue.push(job);
    }

    m_job_queue_cond.notify_one();
}

void nitrocraft::world::ChunkManager::EnqueueDedupJob_ThreadUnsafe(Job job)
{
    if (job.chunk->enqueued_states.fetch_or(JobBit(job.type), std::memory_order_relaxed) & JobBit(job.type)) return;

    m_job_queue.push(job);
}

void nitrocraft::world::ChunkManager::GenerationJobHandler(Chunk* chunk)
{
    // Called chunk is in stage==Empty -> ready for terrain/cave generation.
    auto expected = ChunkStage::Empty;
    if (!chunk->stage.compare_exchange_strong(expected, ChunkStage::GenerationInProgress, std::memory_order_acq_rel, std::memory_order_acquire)) return;

    m_terrain_generator.GenerateTerrain(chunk);

    chunk->stage.store(ChunkStage::GenerationComplete, std::memory_order_release);
}

void nitrocraft::world::ChunkManager::LocalLightingJobHandler(Chunk* chunk)
{
    // Called chunk has to be in stage==GenerationComplete state 
    if (chunk->stage.load(std::memory_order_acquire) < ChunkStage::GenerationComplete)
    {
        {
            std::lock_guard<std::mutex> lock{ m_job_queue_mutex };

            EnqueueDedupJob_ThreadUnsafe({ chunk, JobType::Generation });
            EnqueueDedupJob_ThreadUnsafe({ chunk, JobType::LocalLighting });
        }

        m_job_queue_cond.notify_one();

        return;
    }

    // Called chunk's neighbours have to be in stage>=GenerationComplete state
    {
        std::array<Chunk*, static_cast<std::size_t>(ChunkNeighbour::COUNT)> missings{};
        auto missings_iter = missings.begin();

        for (auto neighbour : chunk->neighbours)
        {
            if (neighbour->stage.load(std::memory_order_acquire) >= ChunkStage::GenerationComplete) continue;

            *missings_iter = neighbour;
            missings_iter++;
        }

        {
            std::lock_guard<std::mutex> lock{ m_job_queue_mutex };

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
            m_job_queue_cond.notify_one();
            return;
        }
    }

    // Above conditionas are met -> start propagating local lights
    ChunkStage expected = ChunkStage::GenerationComplete;
    if (!chunk->stage.compare_exchange_strong(expected, ChunkStage::LocalLightingInProgress, std::memory_order_acq_rel, std::memory_order_acquire)) return;

    world::PropagateInitialSunlight(chunk);

    chunk->stage.store(ChunkStage::LocalLightingComplete, std::memory_order_release);
}

void nitrocraft::world::ChunkManager::NeighbourLightingJobHandler(Chunk* chunk)
{
    // Called chunk has to be in stage==LocalLightingComplete.
    if (chunk->stage.load(std::memory_order_acquire) < ChunkStage::LocalLightingComplete)
    {
        {
            std::lock_guard<std::mutex> lock{ m_job_queue_mutex };

            EnqueueDedupJob_ThreadUnsafe({ chunk, JobType::LocalLighting });
            EnqueueDedupJob_ThreadUnsafe({ chunk, JobType::NeighbourLighting});
        }

        m_job_queue_cond.notify_one();

        return;
    }

    // Called chunk's neighbours have to be in stage>=LocalLightingComplete state.
    {
        std::array<Chunk*, static_cast<std::size_t>(ChunkNeighbour::COUNT)> missings{};
        auto missings_iter = missings.begin();

        for (auto neighbour : chunk->neighbours)
        {
            if (neighbour->stage.load(std::memory_order_acquire) >= ChunkStage::LocalLightingComplete) continue;

            *missings_iter = neighbour;
            missings_iter++;
        }

        {
            std::lock_guard<std::mutex> lock{ m_job_queue_mutex };

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
            m_job_queue_cond.notify_one();
            return;
        }
    }

    // Above conditions are met -> called chunk's lighting is complete (for now).
    ChunkStage expected = ChunkStage::LocalLightingComplete;
    if (!chunk->stage.compare_exchange_strong(expected, ChunkStage::NeighbourLightingInProgress, std::memory_order_acq_rel, std::memory_order_acquire)) return;

    // TODO: neighbour light propagation

    chunk->storage_version.fetch_add(1, std::memory_order_relaxed);

    chunk->stage.store(ChunkStage::NeighbourLightingComplete, std::memory_order_release);
}
