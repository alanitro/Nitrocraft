#pragma once

#include <utility>
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>
#include "Graphics_Shader.hpp"
#include "Graphics_Mesh.hpp"
#include "World_Coordinate.hpp"
#include "World_ChunkManager.hpp"

class Camera;
struct World_Chunk;

class Graphics_WorldRenderer
{
public:
    Graphics_WorldRenderer() = default;
    ~Graphics_WorldRenderer() = default;

    void Initialize();
    void Terminate();

    void Render(const Camera& camera, float sunlight_intensity, glm::vec3 sky_color);

    void PrepareChunksToRender(const std::vector<World_Chunk*>& chunks_in_render_area);

    void EnableAmbientOcclusion(bool enable)
    {
        static bool prev_enable = m_EnableAmbientOcclusion;

        if (enable != prev_enable)
        {
            for (auto& handle : m_ChunkGPUMeshHandles)
            {
                handle.second.UploadedVersion = 0;
                handle.second.RequestedVersion = 0;
            }
        }

        prev_enable = m_EnableAmbientOcclusion;

        m_EnableAmbientOcclusion = enable;
    }

private:
    // Graphics Pipeline
    GLuint m_BlockTextureAtlas = 0;

    Graphics_Shader m_ChunkShader;

    // Mesh storage
    struct GPUMeshHandleHolder
    {
        std::unique_ptr<Graphics_ChunkGPUMeshHandle> Handle;
        std::uint32_t UploadedVersion = 0;
        std::uint32_t RequestedVersion = 0;
    };

    bool m_EnableAmbientOcclusion = true;

    std::vector<World_Chunk_ID> m_GPUMeshIDsToRender;
    std::unordered_map<World_Chunk_ID, GPUMeshHandleHolder> m_ChunkGPUMeshHandles;

    // Meshing threads
    std::vector<std::jthread>  m_MeshingThreads;
    std::size_t                m_MeshingThreadCount = 0;

    // Meshing job
    struct MeshingJob
    {
        World_Chunk*  MeshingChunk;
        std::uint32_t RequestVersion;
    };
    std::queue<MeshingJob>  m_MeshingJobQueue;
    bool                    m_MeshingJobRetire = false;
    std::mutex              m_MeshingJobMutex;
    std::condition_variable m_MeshingJobCond;

    std::queue<Graphics_ChunkCPUMesh> m_CompletedCPUMeshQueue;
    std::mutex                        m_CompletedCPUMeshQueueMutex;

    void CPUMeshingWorkLoop();
};
