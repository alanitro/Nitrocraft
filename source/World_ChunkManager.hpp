#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <queue>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "World_Coordinate.hpp"
#include "World_Chunk.hpp"
#include "World_TerrainGenerator.hpp"

namespace nitrocraft::world
{

class ChunkManager
{
public:
    void Initialize();
    void Terminate();

    // Called from main thread per frame.
    void SetCenterChunk_MainThread(ChunkID center_id);

    std::vector<Chunk*> GetChunksInRenderArea_MainThread() const;

    // Queries
    std::size_t GetRenderDistance()     const { return m_render_distance; }
    std::size_t GetWorkerThreadCount()  const { return m_worker_count; }
    std::size_t GetLoadedChunkCount()   const { std::lock_guard<std::mutex> lock{ m_chunk_map_mutex }; return m_chunk_map.size(); }

    std::optional<const Chunk*> GetChunkAt(GlobalXYZ global) const;

    // Modifiers
    void SetRenderDistance(std::size_t render_distance);

private:
    std::size_t m_render_distance = 6;

    std::size_t GetLoadingDistance() const;
    std::size_t GetLoadingDiameter() const;

    ChunkID m_current_chunk_id{ -1, -1, -1 };

    std::unordered_map<ChunkID, std::unique_ptr<Chunk>> m_chunk_map;
    mutable std::mutex m_chunk_map_mutex;

    // Chunk construction job system
    enum class JobType
    {
        Generation,
        LocalLighting,
        NeighbourLighting,
    };

    static constexpr std::uint8_t JobBit(JobType type)
    {
        return static_cast<std::uint8_t>(1u << static_cast<std::uint8_t>(type));
    }

    struct Job
    {
        Chunk*  chunk;
        JobType type;
    };

    std::queue<Job>         m_job_queue; // TODO: Use priority queue
    std::atomic<bool>       m_job_retire = false;
    std::mutex              m_job_queue_mutex;
    std::condition_variable m_job_queue_cond;

    std::vector<std::jthread> m_workers;
    std::size_t m_worker_count = 0;

    TerrainGenerator m_terrain_generator;

    void JobLoop();

    // Called from worker thread
    void EnqueueDedupJob_ThreadSafeWithNotify(Job job);
    void EnqueueDedupJob_ThreadUnsafe(Job job);
    void GenerationJobHandler(Chunk* chunk);
    void LocalLightingJobHandler(Chunk* chunk);
    void NeighbourLightingJobHandler(Chunk* chunk);
};

} // namespace nitrocraft::world
