#pragma once

#include <utility>
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>
#include "Graphics_Shader.hpp"
#include "Graphics_Mesh.hpp"
#include "World_Coordinate.hpp"
#include "World_Chunk.hpp"
#include "World_ChunkManager.hpp"

class Camera;

namespace nitrocraft::graphics
{

class WorldRenderer
{
public:
    WorldRenderer() = default;
    ~WorldRenderer() = default;

    void Initialize();
    void Terminate();

    void Render(const Camera& camera, float sunlight_intensity, glm::vec3 sky_color);

    void PrepareChunksToRender(const std::vector<world::Chunk*>& chunks_in_render_area);

    void EnableAmbientOcclusion(bool enable);

private:
    // Graphics Pipeline
    GLuint m_block_texture_atlas = 0;

    Shader m_chunk_shader;

    // Mesh storage
    struct GPUMeshHandleHolder
    {
        std::unique_ptr<ChunkGPUMeshHandle> handle;
        std::uint32_t uploaded_version = 0;
        std::uint32_t requested_version = 0;
    };

    bool m_enable_ambient_occlusion = true;

    std::vector<world::ChunkID> m_gpumesh_ids_to_render;
    std::unordered_map<world::ChunkID, GPUMeshHandleHolder> m_chunk_gpumesh_handles;

    // Meshing threads
    std::vector<std::jthread>  m_meshing_threads;
    std::size_t                m_meshing_thread_count = 0;

    // Meshing job
    struct MeshingJob
    {
        world::Chunk* meshing_chunk;
        std::uint32_t requested_version;
    };

    std::queue<MeshingJob>  m_meshing_job_queue;
    std::mutex              m_meshing_job_mutex;
    std::condition_variable m_meshing_job_cond;
    bool                    m_meshing_job_retire = false;

    std::queue<ChunkCPUMesh> m_completed_cpumesh_queue;
    std::mutex               m_completed_cpumesh_queue_mutex;

    void CPUMeshingWorkLoop();
};

} // namespace nitrocraft::graphics
