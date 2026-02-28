#include "Graphics_BlockOutlineRenderer.hpp"

#include <print>
#include <glad/gl.h>
#include "Graphics_Shader.hpp"
#include "Utility_IO.hpp"

namespace
{
    struct ChunkOutlineGPUMeshHandle
    {
        GLuint VertexArrayID;
        GLuint VertexBufferID;
    };

    ChunkOutlineGPUMeshHandle GPUMeshHandle;

    Graphics_Shader ChunkOutlineShader;

    const float ChunkOutlineVertices[] =
    {
         0.0f,   0.0f,  0.0f,   16.0f,   0.0f,  0.0f,
        16.0f,   0.0f,  0.0f,   16.0f, 256.0f,  0.0f,
        16.0f, 256.0f,  0.0f,    0.0f, 256.0f,  0.0f,
         0.0f, 256.0f,  0.0f,    0.0f,   0.0f,  0.0f,

         0.0f,   0.0f, 16.0f,   16.0f,   0.0f, 16.0f,
        16.0f,   0.0f, 16.0f,   16.0f, 256.0f, 16.0f,
        16.0f, 256.0f, 16.0f,    0.0f, 256.0f, 16.0f,
         0.0f, 256.0f, 16.0f,    0.0f,   0.0f, 16.0f,

         0.0f,   0.0f,  0.0f,    0.0f,   0.0f, 16.0f,
        16.0f,   0.0f,  0.0f,   16.0f,   0.0f, 16.0f,
        16.0f, 256.0f,  0.0f,   16.0f, 256.0f, 16.0f,
         0.0f, 256.0f,  0.0f,    0.0f, 256.0f, 16.0f,
    };
}

void Graphics_ChunkOutlineRenderer_Initialize()
{
    glGenVertexArrays(1, &GPUMeshHandle.VertexArrayID);
    glGenBuffers(1, &GPUMeshHandle.VertexBufferID);

    glBindVertexArray(GPUMeshHandle.VertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, GPUMeshHandle.VertexBufferID);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, reinterpret_cast<const void*>(0));
    glEnableVertexAttribArray(0);

    glBufferData(GL_ARRAY_BUFFER, sizeof(ChunkOutlineVertices), ChunkOutlineVertices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    auto vshader_source_opt = IO_ReadFile("resource/shader/ChunkOutline.vert.glsl");
    if (vshader_source_opt.has_value() == false)
    {
        std::println("Error: Failed to load resource/shader/ChunkOutline.vert.glsl.");
        return;
    }

    auto fshader_source_opt = IO_ReadFile("resource/shader/ChunkOutline.frag.glsl");
    if (fshader_source_opt.has_value() == false)
    {
        std::println("Error: Failed to load resource/shader/ChunkOutline.frag.glsl.");
        return;
    }

    ChunkOutlineShader.Create(vshader_source_opt.value(), fshader_source_opt.value());
}

void Graphics_ChunkOutlineRenderer_Terminate()
{
    glDeleteVertexArrays(1, &GPUMeshHandle.VertexArrayID);
    glDeleteBuffers(1, &GPUMeshHandle.VertexBufferID);

    ChunkOutlineShader.Destroy();
}

void Graphics_ChunkOutlineRenderer_Render(const Camera& camera, World_Position chunk_offset)
{
    ChunkOutlineShader.Use();
    ChunkOutlineShader.SetUniform("u_ChunkPosition", chunk_offset);
    ChunkOutlineShader.SetUniform("u_MVP", camera.GetViewProjection());

    glBindVertexArray(GPUMeshHandle.VertexArrayID);

    glDrawArrays(GL_LINES, 0, sizeof(ChunkOutlineVertices) / (sizeof(float) * 3));

    glBindVertexArray(0);
}
