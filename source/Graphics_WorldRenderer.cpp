#include "Graphics_WorldRenderer.hpp"

#include <memory>
#include <array>
#include <unordered_map>
#include <algorithm>
#include <print>
#include <glm/vec2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>
#include "Utility_IO.hpp"
#include "Graphics_Camera.hpp"
#include "Graphics_Shader.hpp"
#include "Graphics_Mesh.hpp"
#include "World_Coordinate.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"
#include "Utility_Time.hpp"

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
    m_MeshingThreadCount = std::clamp<std::size_t>(std::thread::hardware_concurrency() / 2 - 1, 1u, 4u);

    m_MeshingThreads.reserve(m_MeshingThreadCount);

    for (std::size_t i = 0; i < m_MeshingThreadCount; i++)
    {
        m_MeshingThreads.emplace_back(
            [this]()
            {

            }
        );
    }
}

void Graphics_WorldRenderer::Terminate()
{
    m_ChunkGPUMeshHandles.clear();

    m_ChunkShader.Destroy();

    glDeleteTextures(1, &m_BlockTextureAtlas);
}

void Graphics_WorldRenderer::Render(const Camera& camera, float sunlight_intensity, glm::vec3 sky_color)
{
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

    for (auto& chunk_id : m_GPUMeshIDsToRender)
    {
        auto it = m_ChunkGPUMeshHandles.find(chunk_id);

        if (it == m_ChunkGPUMeshHandles.end()) continue;

        auto& handle = it->second;

        glBindVertexArray(handle->VertexArrayID);

        glDrawElements(GL_TRIANGLES, handle->IndicesCount, GL_UNSIGNED_INT, reinterpret_cast<const void*>(0));
    }

    m_GPUMeshIDsToRender.clear();
}

void Graphics_WorldRenderer::PrepareChunksToRender(const std::vector<World_Chunk*>& chunks_in_render_area)
{
    for (auto chunk : chunks_in_render_area)
    {
        World_Chunk_ID chunk_id = chunk->ID;
        World_GlobalXYZ chunk_offset = World_FromChunkIDToChunkOffset(chunk_id);

        // 1) Choose chunks to be rendered.
        // 2-1) Check if the chosen chunk has old mesh handle.
        // 3-1) Check if the chosen chunk has higher storage version than mesh handle version. If 
        // 2) Check if gpu mesh handle exists for a chosen chunks
        // 3-1) If handle exists and is the same version then use that mesh
        // 3-2) If handle exists and is not the same version then make new handle and upload
        // 3-3) If handle DNE then make new handle and upload

        static Graphics_ChunkCPUMesh cpumesh;
        cpumesh.Vertices.reserve(World_CHUNK_VOLUME * 6 * 4);
        cpumesh.Indices.reserve(World_CHUNK_VOLUME * 6 * 6);
        cpumesh.Vertices.clear();
        cpumesh.Indices.clear();

        if (auto iter = m_ChunkGPUMeshHandles.find(chunk_id); iter != m_ChunkGPUMeshHandles.end())
        {
            if (auto storage_version = chunk->StorageVersion.load(std::memory_order_acquire); storage_version > iter->second->Version)
            {
                m_ChunkGPUMeshHandles.erase(iter);

                Graphics_Mesh_GenerateChunkCPUMesh_AmbientOcclusion(chunk, cpumesh);

                auto handle = std::make_unique<Graphics_ChunkGPUMeshHandle>(storage_version);

                handle->UploadCPUMeshToGPU(cpumesh);

                m_ChunkGPUMeshHandles.emplace(chunk_id, std::move(handle));
            }

            m_GPUMeshIDsToRender.push_back(chunk_id);
        }
        else
        {
            if (chunk->Stage.load(std::memory_order_acquire) == World_Chunk_Stage::NeighbourLightingComplete)
            {
                Graphics_Mesh_GenerateChunkCPUMesh_AmbientOcclusion(chunk, cpumesh);

                auto handle = std::make_unique<Graphics_ChunkGPUMeshHandle>(chunk->StorageVersion.load(std::memory_order_acquire));

                handle->UploadCPUMeshToGPU(cpumesh);

                m_ChunkGPUMeshHandles.emplace(chunk_id, std::move(handle));

                m_GPUMeshIDsToRender.push_back(chunk_id);
            }
        }
    }
}
