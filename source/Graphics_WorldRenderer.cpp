#include "Graphics_WorldRenderer.hpp"

#include "Graphics_Camera.hpp"
#include "Graphics_Mesh.hpp"
#include "Graphics_Shader.hpp"
#include "Utility_IO.hpp"
#include "Utility_Time.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"
#include "World_Coordinate.hpp"
#include <algorithm>
#include <array>
#include <glad/gl.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vec2.hpp>
#include <memory>
#include <unordered_map>
#include <print>

namespace nitrocraft::graphics
{

void WorldRenderer::Initialize()
{
    // Load shader program
    auto vshader_source_opt = utility::ReadFile("resource/shader/Chunk.vert.glsl");
    if (vshader_source_opt.has_value() == false)
    {
        std::println("Error: Failed to load resource/shader/Chunk.vert.glsl.");
        return;
    }

    auto fshader_source_opt = utility::ReadFile("resource/shader/Chunk.frag.glsl");
    if (fshader_source_opt.has_value() == false)
    {
        std::println("Error: Failed to load resource/shader/Chunk.frag.glsl.");
        return;
    }

    m_chunk_shader.Create(vshader_source_opt.value(), fshader_source_opt.value());

    // Load texture atlas
    auto image_opt = utility::ReadImage("./resource/texture/Blocks.png", true);

    if (image_opt.has_value() == false)
    {
        std::println("Error: Failed to load ./resource/texture/Blocks.png");
        return;
    }

    auto& image = image_opt.value();

    GLint format = (image.channel_numbers == 4) ? GL_RGBA : GL_RGB;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, image.data.data());
    //glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_block_texture_atlas = texture;

    // Start meshing worker threads
    m_meshing_thread_count = std::clamp<std::size_t>(std::max(1u, std::thread::hardware_concurrency()) / 2, 1u, 8u);

    m_meshing_threads.reserve(m_meshing_thread_count);

    for (std::size_t i = 0; i < m_meshing_thread_count; i++)
    {
        m_meshing_threads.emplace_back(
            [this]()
            {
                this->CPUMeshingWorkLoop();
            }
        );
    }
}

void WorldRenderer::Terminate()
{
    {
        std::lock_guard<std::mutex> lock{ m_meshing_job_mutex };
        m_meshing_job_retire = true;
    }
    m_meshing_job_cond.notify_all();

    for (auto& t : m_meshing_threads)
    {
        if (t.joinable()) t.join();
    }
    m_meshing_threads.clear();

    m_chunk_gpumesh_handles.clear();

    m_chunk_shader.Destroy();

    glDeleteTextures(1, &m_block_texture_atlas);
}

void WorldRenderer::Render(const Camera& camera, float sunlight_intensity, glm::vec3 sky_color)
{
    // Config pipeline
    glClearColor(sky_color.r, sky_color.g, sky_color.b, 1.0f);

    m_chunk_shader.Use();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_block_texture_atlas);

    m_chunk_shader.SetUniform("u_Texture", 0);
    m_chunk_shader.SetUniform("u_ModelViewProjection", camera.GetViewProjection());
    m_chunk_shader.SetUniform("u_SunlightIntensity", sunlight_intensity);

    // Render chunk mesh
    for (auto& chunk_id : m_gpumesh_ids_to_render)
    {
        auto it = m_chunk_gpumesh_handles.find(chunk_id);

        if (it == m_chunk_gpumesh_handles.end()) continue;

        auto& holder = it->second;

        if (!holder.handle) continue;

        glBindVertexArray(holder.handle->vertex_array_id);

        glDrawElements(GL_TRIANGLES, holder.handle->indices_count, GL_UNSIGNED_INT, reinterpret_cast<const void*>(0));
    }

    glBindVertexArray(0);

    m_gpumesh_ids_to_render.clear();
}

void WorldRenderer::PrepareChunksToRender(const std::vector<world::Chunk*>& chunks_in_render_area)
{
    // Queue missing chunk cpu mesh
    for (auto chunk : chunks_in_render_area)
    {
        if (chunk->stage.load(std::memory_order_acquire) < world::ChunkStage::NeighbourLightingComplete) continue;

        m_gpumesh_ids_to_render.emplace_back(chunk->id);

        auto chunk_storage_version = chunk->storage_version.load(std::memory_order_acquire);

        GPUMeshHandleHolder* holder = nullptr;

        if (auto iter = m_chunk_gpumesh_handles.find(chunk->id); iter == m_chunk_gpumesh_handles.end())
        {
            auto [new_iter, res] = m_chunk_gpumesh_handles.emplace(chunk->id, GPUMeshHandleHolder{ std::make_unique<ChunkGPUMeshHandle>(), 0, 0});
            holder = &new_iter->second;
        }
        else
        {
            holder = &iter->second;
        }

        if (holder->uploaded_version < chunk_storage_version && holder->requested_version < chunk_storage_version)
        {
            // Push to mesh gen queue
            {
                std::lock_guard<std::mutex> lock{ m_meshing_job_mutex };
                m_meshing_job_queue.emplace(chunk, chunk_storage_version);
            }
            m_meshing_job_cond.notify_one();

            holder->requested_version = chunk_storage_version;
        }
    }

    // Upload completed mesh to gpu
    while (true)
    {
        // Get completed cpumesh
        ChunkCPUMesh cpumesh;

        {
            std::lock_guard<std::mutex> lock{ m_completed_cpumesh_queue_mutex };

            if (m_completed_cpumesh_queue.empty()) break;

            cpumesh = std::move(m_completed_cpumesh_queue.front()); m_completed_cpumesh_queue.pop();
        }

        if (cpumesh.meshed_chunk->storage_version.load(std::memory_order_acquire) > cpumesh.completed_version) continue;

        // Upload finished cpumesh to gpu if its version is > gpu mesh version.
        auto iter = m_chunk_gpumesh_handles.find(cpumesh.meshed_chunk->id);

        if (iter == m_chunk_gpumesh_handles.end()) continue;

        auto& holder = iter->second;

        if (cpumesh.completed_version != holder.requested_version) continue;

        holder.uploaded_version = holder.requested_version;

        glBindBuffer(GL_ARRAY_BUFFER, holder.handle->vertex_buffer_id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, holder.handle->index_buffer_id);

        glBufferData(GL_ARRAY_BUFFER, cpumesh.vertices.size() * sizeof(ChunkMeshVertexLayout), cpumesh.vertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, cpumesh.indices.size() * sizeof(std::uint32_t), cpumesh.indices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        holder.handle->indices_count = static_cast<std::uint32_t>(cpumesh.indices.size());
    }
}

void WorldRenderer::EnableAmbientOcclusion(bool enable)
{
    static bool prev_enable = m_enable_ambient_occlusion;

    if (enable != prev_enable)
    {
        for (auto& handle : m_chunk_gpumesh_handles)
        {
            handle.second.uploaded_version = 0;
            handle.second.requested_version = 0;
        }
    }

    prev_enable = m_enable_ambient_occlusion;

    m_enable_ambient_occlusion = enable;
}

void WorldRenderer::CPUMeshingWorkLoop()
{
    while (true)
    {
        world::Chunk*  chunk = nullptr;
        std::uint32_t request_version = 0;

        {
            std::unique_lock<std::mutex> lock{ m_meshing_job_mutex };

            m_meshing_job_cond.wait(lock, [this]() { return m_meshing_job_retire || !m_meshing_job_queue.empty(); });

            if (m_meshing_job_retire) return;

            auto [c,v] = m_meshing_job_queue.front(); m_meshing_job_queue.pop();
            chunk = c;
            request_version = v;
        }

        if (request_version < chunk->storage_version.load(std::memory_order_acquire)) continue;

        auto cpumesh =
            (m_enable_ambient_occlusion) ?
            GenerateChunkCPUMesh_AmbientOcclusion(chunk) :
            GenerateChunkCPUMesh(chunk);

        cpumesh.completed_version = request_version;

        {
            std::lock_guard<std::mutex> lock{ m_completed_cpumesh_queue_mutex };

            m_completed_cpumesh_queue.emplace(std::move(cpumesh));
        }
    }
}

} // namespace nitrocraft::graphics
