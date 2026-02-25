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

void Graphics_WorldRenderer::Initialize()
{
    // Load shader program
    auto vshader_source_opt = IO_ReadFile("resource/shader/Chunk.vert.glsl");
    if (vshader_source_opt.has_value() == false)
    {
        std::println("Error: Failed to load resource/shader/Chunk.vert.glsl.");
        return;
    }

    auto fshader_source_opt = IO_ReadFile("resource/shader/Chunk.frag.glsl");
    if (fshader_source_opt.has_value() == false)
    {
        std::println("Error: Failed to load resource/shader/Chunk.frag.glsl.");
        return;
    }

    m_ChunkShader.Create(vshader_source_opt.value(), fshader_source_opt.value());

    // Load texture atlas
    auto image_opt = IO_ReadImage("./resource/texture/Blocks.png", true);

    if (image_opt.has_value() == false)
    {
        std::println("Error: Failed to load ./resource/texture/Blocks.png");
        return;
    }

    auto& image = image_opt.value();

    GLint format = (image.ChannelNumbers == 4) ? GL_RGBA : GL_RGB;

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, format, image.Width, image.Height, 0, format, GL_UNSIGNED_BYTE, image.ImageData.data());
    //glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    m_BlockTextureAtlas = texture;

    // Start meshing worker threads
    m_MeshingThreadCount = std::clamp<std::size_t>(std::max(1u, std::thread::hardware_concurrency()) / 2, 1u, 8u);

    m_MeshingThreads.reserve(m_MeshingThreadCount);

    for (std::size_t i = 0; i < m_MeshingThreadCount; i++)
    {
        m_MeshingThreads.emplace_back(
            [this]()
            {
                this->CPUMeshingWorkLoop();
            }
        );
    }
}

void Graphics_WorldRenderer::Terminate()
{
    {
        std::lock_guard<std::mutex> lock{ m_MeshingJobMutex };
        m_MeshingJobRetire = true;
    }
    m_MeshingJobCond.notify_all();

    for (auto& t : m_MeshingThreads)
    {
        if (t.joinable()) t.join();
    }
    m_MeshingThreads.clear();

    m_ChunkGPUMeshHandles.clear();

    m_ChunkShader.Destroy();

    glDeleteTextures(1, &m_BlockTextureAtlas);
}

void Graphics_WorldRenderer::Render(const Camera& camera, float sunlight_intensity, glm::vec3 sky_color)
{
    // Config pipeline
    glClearColor(sky_color.r, sky_color.g, sky_color.b, 1.0f);

    m_ChunkShader.Use();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_BlockTextureAtlas);

    m_ChunkShader.SetUniform("u_Texture", 0);
    m_ChunkShader.SetUniform("u_ModelViewProjection", camera.GetViewProjection());
    m_ChunkShader.SetUniform("u_SunlightIntensity", sunlight_intensity);

    // Render chunk mesh
    for (auto& chunk_id : m_GPUMeshIDsToRender)
    {
        auto it = m_ChunkGPUMeshHandles.find(chunk_id);

        if (it == m_ChunkGPUMeshHandles.end()) continue;

        auto& holder = it->second;

        if (!holder.Handle) continue;

        glBindVertexArray(holder.Handle->VertexArrayID);

        glDrawElements(GL_TRIANGLES, holder.Handle->IndicesCount, GL_UNSIGNED_INT, reinterpret_cast<const void*>(0));
    }

    glBindVertexArray(0);

    m_GPUMeshIDsToRender.clear();
}

void Graphics_WorldRenderer::PrepareChunksToRender(const std::vector<World_Chunk*>& chunks_in_render_area)
{
    // Queue missing chunk cpu mesh
    for (auto chunk : chunks_in_render_area)
    {
        if (chunk->Stage.load(std::memory_order_acquire) < World_Chunk_Stage::NeighbourLightingComplete) continue;

        m_GPUMeshIDsToRender.emplace_back(chunk->ID);

        auto chunk_storage_version = chunk->StorageVersion.load(std::memory_order_acquire);

        if (auto iter = m_ChunkGPUMeshHandles.find(chunk->ID); iter != m_ChunkGPUMeshHandles.end())
        {
            if (iter->second.UploadedVersion < chunk_storage_version && iter->second.RequestedVersion < chunk_storage_version)
            {
                // Push to mesh gen queue
                {
                    std::lock_guard<std::mutex> lock{ m_MeshingJobMutex };
                    m_MeshingJobQueue.emplace(chunk, chunk_storage_version);
                }
                m_MeshingJobCond.notify_one();

                iter->second.RequestedVersion = chunk_storage_version;
            }
        }
        else
        {
            // Push to mesh gen queue
            {
                std::lock_guard<std::mutex> lock{ m_MeshingJobMutex };
                m_MeshingJobQueue.emplace(chunk, chunk_storage_version);
            }
            m_MeshingJobCond.notify_one();

            // Create holder
            m_ChunkGPUMeshHandles.emplace(chunk->ID, GPUMeshHandleHolder{ nullptr, 0, chunk_storage_version });
        }
    }

    // Upload completed mesh to gpu
    while (true)
    {
        // Get completed cpumesh
        m_CompletedCPUMeshQueueMutex.lock();

        if (m_CompletedCPUMeshQueue.empty())
        {
            m_CompletedCPUMeshQueueMutex.unlock();
            break;
        }

        Graphics_ChunkCPUMesh cpumesh = std::move(m_CompletedCPUMeshQueue.front()); m_CompletedCPUMeshQueue.pop();

        m_CompletedCPUMeshQueueMutex.unlock();

        if (cpumesh.MeshedChunk->StorageVersion.load(std::memory_order_acquire) > cpumesh.RequestedVersion) continue;

        // Upload finished cpumesh to gpu if its version is > gpu mesh version.
        auto iter = m_ChunkGPUMeshHandles.find(cpumesh.MeshedChunk->ID);

        if (iter == m_ChunkGPUMeshHandles.end()) continue;

        auto& holder = iter->second;

        if (cpumesh.RequestedVersion != holder.RequestedVersion) continue;

        holder.Handle = std::make_unique<Graphics_ChunkGPUMeshHandle>();

        holder.UploadedVersion = cpumesh.RequestedVersion;

        glBindBuffer(GL_ARRAY_BUFFER, holder.Handle->VertexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, holder.Handle->IndexBufferID);

        glBufferData(GL_ARRAY_BUFFER, cpumesh.Vertices.size() * sizeof(Graphics_ChunkMeshVertexLayout), cpumesh.Vertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, cpumesh.Indices.size() * sizeof(std::uint32_t), cpumesh.Indices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        holder.Handle->IndicesCount = static_cast<std::uint32_t>(cpumesh.Indices.size());
    }
}

void Graphics_WorldRenderer::CPUMeshingWorkLoop()
{
    while (true)
    {
        World_Chunk*  chunk = nullptr;
        std::uint32_t request_version = 0;

        {
            std::unique_lock<std::mutex> lock{ m_MeshingJobMutex };

            m_MeshingJobCond.wait(lock, [this]() { return m_MeshingJobRetire || !m_MeshingJobQueue.empty(); });

            if (m_MeshingJobRetire) return;

            auto [c,v] = m_MeshingJobQueue.front(); m_MeshingJobQueue.pop();
            chunk = c;
            request_version = v;
        }

        if (request_version < chunk->StorageVersion.load(std::memory_order_acquire)) continue;

        auto cpumesh = Graphics_Mesh_GenerateChunkCPUMesh_AmbientOcclusion(chunk);

        cpumesh.RequestedVersion = request_version;

        {
            std::lock_guard<std::mutex> lock{ m_CompletedCPUMeshQueueMutex };

            m_CompletedCPUMeshQueue.emplace(std::move(cpumesh));
        }
    }
}
