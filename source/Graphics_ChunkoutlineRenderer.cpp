#include "Graphics_BlockOutlineRenderer.hpp"

#include <print>
#include <glad/gl.h>
#include "Graphics_Shader.hpp"
#include "Utility_IO.hpp"
#include "Graphics_ChunkOutlineRenderer.hpp"

namespace nitrocraft::graphics
{

namespace 
{
    constexpr float s_chunk_outline_vertices[] =
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

void ChunkOutlineRenderer::Initialize()
{
    glGenVertexArrays(1, &m_vertex_array_id);
    glGenBuffers(1, &m_vertex_buffer_id);

    glBindVertexArray(m_vertex_array_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer_id);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, reinterpret_cast<const void*>(0));
    glEnableVertexAttribArray(0);

    glBufferData(GL_ARRAY_BUFFER, sizeof(s_chunk_outline_vertices), s_chunk_outline_vertices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    auto vshader_source_opt = utility::ReadFile("resource/shader/ChunkOutline.vert.glsl");
    if (vshader_source_opt.has_value() == false)
    {
        std::println("Error: Failed to load resource/shader/ChunkOutline.vert.glsl.");
        return;
    }

    auto fshader_source_opt = utility::ReadFile("resource/shader/ChunkOutline.frag.glsl");
    if (fshader_source_opt.has_value() == false)
    {
        std::println("Error: Failed to load resource/shader/ChunkOutline.frag.glsl.");
        return;
    }

    m_chunk_outline_shader.Create(vshader_source_opt.value(), fshader_source_opt.value());
}

void ChunkOutlineRenderer::Terminate()
{
    glDeleteVertexArrays(1, &m_vertex_array_id);
    glDeleteBuffers(1, &m_vertex_buffer_id);

    m_chunk_outline_shader.Destroy();
}

void ChunkOutlineRenderer::Render(const glm::mat4 & model_view_proj, glm::vec3 chunk_offset)
{
    m_chunk_outline_shader.Use();
    m_chunk_outline_shader.SetUniform("u_ChunkPosition", chunk_offset);
    m_chunk_outline_shader.SetUniform("u_MVP", model_view_proj);

    glBindVertexArray(m_vertex_array_id);

    glDrawArrays(GL_LINES, 0, sizeof(s_chunk_outline_vertices) / (sizeof(float) * 3));

    glBindVertexArray(0);
}

} // namespace nitrocraft::graphics
