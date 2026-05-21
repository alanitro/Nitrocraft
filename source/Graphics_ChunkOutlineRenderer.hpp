#pragma once

#include <glad/gl.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace nitrocraft::graphics
{

class ChunkOutlineRenderer
{
public:
    void Initialize();
    void Terminate();

    void Render(const glm::mat4& model_view_proj, glm::vec3 chunk_position);

private:
    GLuint m_vertex_array_id;
    GLuint m_vertex_buffer_id;

    Shader m_chunk_outline_shader;
};

} // namespace nitrocraft::graphics
