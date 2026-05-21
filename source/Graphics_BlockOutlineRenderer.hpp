#pragma once

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glad/gl.h>
#include "Graphics_Shader.hpp"

namespace nitrocraft::graphics
{

class BlockOutlineRenderer
{
public:
    void Initialize();
    void Terminate();

    void Render(const glm::mat4& model_view_proj, glm::vec3 block_position);

private:
    GLuint m_vertex_array_id;
    GLuint m_vertex_buffer_id;

    Shader m_block_outline_shader;
};

} // namespace nitrocraft::graphics
