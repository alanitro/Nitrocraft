#include "Graphics_WorldRenderer.hpp"

#include <memory>
#include <array>
#include <unordered_map>
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

namespace
{
    GLuint BlocksTexture = 0;

    Graphics_Shader ChunkShader;

    std::vector<World_Chunk_ID> GPUMeshIDsToRender;
    std::unordered_map<World_Chunk_ID, std::unique_ptr<Graphics_ChunkGPUMeshHandle>> ChunkGPUMeshHandles;

    void UploadCPUMeshToGPU(World_Chunk* chunk);
    void LoadTexture();

}

void Graphics_WorldRenderer_Initialize()
{
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

    ChunkShader.Create(vshader_source_opt.value(), fshader_source_opt.value());

    LoadTexture();
}

void WorldRenderer_Terminate()
{
    ChunkGPUMeshHandles.clear();

    ChunkShader.Destroy();

    glDeleteTextures(1, &BlocksTexture);
}

void WorldRenderer_Render(const Camera& camera, float sunlight_intensity, glm::vec3 sky_color)
{
    glClearColor(sky_color.r, sky_color.g, sky_color.b, 1.0f);

    ChunkShader.Use();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, BlocksTexture);

    ChunkShader.SetUniform("u_Texture", 0);
    ChunkShader.SetUniform("u_ModelViewProjection", camera.GetViewProjection());
    ChunkShader.SetUniform("u_SunlightIntensity", sunlight_intensity);

    for (auto& chunk_id : GPUMeshIDsToRender)
    {
        auto it = ChunkGPUMeshHandles.find(chunk_id);

        if (it == ChunkGPUMeshHandles.end()) continue;
        
        auto& handle = it->second;

        glBindVertexArray(handle->VertexArrayID);

        glDrawElements(GL_TRIANGLES, handle->IndicesCount, GL_UNSIGNED_INT, reinterpret_cast<const void*>(0));
    }

    GPUMeshIDsToRender.clear();
}

void WorldRenderer_PrepareChunksToRender(const std::vector<World_Chunk*>& chunks_in_render_area)
{
    for (auto c : chunks_in_render_area)
    {
        UploadCPUMeshToGPU(c);
    }
}

namespace
{
    void UploadCPUMeshToGPU(World_Chunk* chunk)
    {
        World_Chunk_ID chunk_id = chunk->ID;
        World_GlobalXYZ chunk_offset = World_FromChunkIDToChunkOffset(chunk_id);

        // If the chunk has completed cpu mesh -> delete the old gpu mesh handle, if it exists.
        // If the chunk does not have completed cpu mesh -> use the old gpu mesh handle, if it exists, exit otherwise.
        World_Chunk_Stage expected = World_Chunk_Stage::MeshingComplete;
        if (chunk->Stage.compare_exchange_strong(expected, World_Chunk_Stage::Ready, std::memory_order_acq_rel, std::memory_order_acquire))
        {
            if (auto iter = ChunkGPUMeshHandles.find(chunk_id); iter != ChunkGPUMeshHandles.end())
            {
                ChunkGPUMeshHandles.erase(chunk_id);
            }

            // TODO: use buffer orphaning instead of recreating new buffer
            // Create new gpu mesh handle.
            auto handle = std::make_unique<Graphics_ChunkGPUMeshHandle>(0);

            auto cpumesh = std::move(chunk->CPUMesh);

            handle->UploadCPUMeshToGPU(cpumesh);

            auto res = ChunkGPUMeshHandles.emplace(chunk_id, std::move(handle));

            GPUMeshIDsToRender.push_back(chunk_id);
        }
        else
        {
            if (auto iter = ChunkGPUMeshHandles.find(chunk_id); iter != ChunkGPUMeshHandles.end())
            {
                GPUMeshIDsToRender.push_back(chunk_id);
            }
        }
    }

    void LoadTexture()
    {
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

        BlocksTexture = texture;
    }
}
