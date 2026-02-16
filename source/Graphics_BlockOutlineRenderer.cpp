#include "Graphics_BlockOutlineRenderer.hpp"

#include <print>
#include <glad/gl.h>
#include "Graphics_Shader.hpp"
#include "Utility_IO.hpp"

namespace
{
    struct BlockOutlineGPUMeshHandle
    {
        GLuint VertexArrayID;
        GLuint VertexBufferID;
    };

    BlockOutlineGPUMeshHandle GPUMeshHandle;

    Graphics_Shader BlockOutlineShader;

    float BlockOutlineVertices[]
    {
        -0.004f, -0.004f, -0.004f,
         1.004f, -0.004f, -0.004f,

         1.004f, -0.004f, -0.004f,
         1.004f,  1.004f, -0.004f,

         1.004f,  1.004f, -0.004f,
        -0.004f,  1.004f, -0.004f,

        -0.004f,  1.004f, -0.004f,
        -0.004f, -0.004f, -0.004f,

        -0.004f, -0.004f, -0.004f,
        -0.004f, -0.004f,  1.004f,

         1.004f, -0.004f, -0.004f,
         1.004f, -0.004f,  1.004f,

         1.004f,  1.004f, -0.004f,
         1.004f,  1.004f,  1.004f,

        -0.004f,  1.004f, -0.004f,
        -0.004f,  1.004f,  1.004f,

        -0.004f, -0.004f,  1.004f,
         1.004f, -0.004f,  1.004f,

         1.004f, -0.004f,  1.004f,
         1.004f,  1.004f,  1.004f,

         1.004f,  1.004f,  1.004f,
        -0.004f,  1.004f,  1.004f,

        -0.004f,  1.004f,  1.004f,
        -0.004f, -0.004f,  1.004f,
    };
}

void Graphics_BlockOutlineRenderer_Initialize()
{
    glGenVertexArrays(1, &GPUMeshHandle.VertexArrayID);
    glGenBuffers(1, &GPUMeshHandle.VertexBufferID);

    glBindVertexArray(GPUMeshHandle.VertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, GPUMeshHandle.VertexBufferID);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, reinterpret_cast<const void*>(0));
    glEnableVertexAttribArray(0);

    glBufferData(GL_ARRAY_BUFFER, sizeof(BlockOutlineVertices), BlockOutlineVertices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


    auto vshader_source_opt = IO_ReadFile("resource/shader/BlockOutline.vert.glsl");
    if (vshader_source_opt.has_value() == false)
    {
        std::println("Error: Failed to load resource/shader/BlockOutline.vert.glsl.");
        return;
    }

    auto fshader_source_opt = IO_ReadFile("resource/shader/BlockOutline.frag.glsl");
    if (fshader_source_opt.has_value() == false)
    {
        std::println("Error: Failed to load resource/shader/BlockOutline.frag.glsl.");
        return;
    }

    BlockOutlineShader.Create(vshader_source_opt.value(), fshader_source_opt.value());
}

void Graphics_BlockOutlineRenderer_Terminate()
{
    glDeleteVertexArrays(1, &GPUMeshHandle.VertexArrayID);
    glDeleteBuffers(1, &GPUMeshHandle.VertexBufferID);

    BlockOutlineShader.Destroy();
}

void Graphics_BlockOutlineRenderer_RenderBlock(const Camera& camera, World_Position block_position)
{
    BlockOutlineShader.Use();
    BlockOutlineShader.SetUniform("u_BlockPosition", block_position);
    BlockOutlineShader.SetUniform("u_MVP", camera.GetViewProjection());

    glBindVertexArray(GPUMeshHandle.VertexArrayID);

    glDrawArrays(GL_LINES, 0, sizeof(BlockOutlineVertices) / (sizeof(float) * 3));

    glBindVertexArray(0);
}
