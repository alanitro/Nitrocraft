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

class World_ChunkManager
{
public:
    World_ChunkManager();
    ~World_ChunkManager();

    // Called from main thread per frame.
    void SetCenterChunkMainThread(World_ChunkID center_id);

    std::vector<World_Chunk*> GetChunksInRenderArea() const;

    // Queries
    std::size_t GetWorkerThreadCount() const { return m_WorkerCount; }
    std::size_t GetLoadedChunkCount() const { std::lock_guard<std::mutex> lock{ m_ChunkMapMutex }; return m_ChunkMap.size(); };

    std::optional<const World_Chunk*> GetChunkAt(World_GlobalXYZ global) const;

    // TODO: Modifiers

private:
    World_ChunkID m_CurrentCenterID{ -1, -1, -1 };

    std::unordered_map<World_ChunkID, std::unique_ptr<World_Chunk>> m_ChunkMap;
    mutable std::mutex m_ChunkMapMutex;

    // Chunk construction job system
    enum class JobType
    {
        Generation,
        LocalLighting,
        NeighbourLighting,
        Meshing,
    };

    static constexpr std::uint8_t JobBit(JobType type)
    {
        return static_cast<std::uint8_t>(1u << static_cast<std::uint8_t>(type));
    }

    struct Job
    {
        World_Chunk* Chunk;
        JobType      Type;
    };

    std::queue<Job>         m_JobQueue; // TODO: Use priority queue
    std::atomic<bool>       m_JobRetire = false;
    std::mutex              m_JobQueueMutex;
    std::condition_variable m_JobQueueCond;

    std::vector<std::jthread> m_Workers;
    std::size_t m_WorkerCount = 0;

    void JobLoop();

    // Called from worker thread
    void EnqueueDedupJob_ThreadSafeWithNotify(Job job);
    void EnqueueDedupJob_ThreadUnsafe(Job job);
    void GenerationJobHandler(World_Chunk* chunk);
    void LocalLightingJobHandler(World_Chunk* chunk);
    void NeighbourLightingJobHandler(World_Chunk* chunk);
    void MeshingJobHandler(World_Chunk* chunk);
};
