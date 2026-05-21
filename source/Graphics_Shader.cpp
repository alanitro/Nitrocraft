#include "Graphics_Shader.hpp"

#include <cassert>
#include <print>
#include <glm/gtc/type_ptr.hpp>

namespace nitrocraft::graphics
{

namespace
{
    GLuint LoadShaderProgram(const std::string& vshader_source, const std::string& fshader_source)
    {
        const char* vshader_source_raw = vshader_source.c_str();
        const char* fshader_source_raw = fshader_source.c_str();

        // Create vertex shader.
        GLint compile_status;
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vshader_source_raw, nullptr);
        glCompileShader(vertex_shader);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
        if (compile_status == GL_FALSE)
        {
            char info_log[512];
            glGetShaderInfoLog(vertex_shader, sizeof(info_log), nullptr, info_log);
            std::println("Error: Vertex Shader: {}", info_log);
            glDeleteShader(vertex_shader);
            return 0;
        }

        // Create fragment shader.
        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fshader_source_raw, nullptr);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_status);
        if (compile_status == GL_FALSE)
        {
            char info_log[512];
            glGetShaderInfoLog(fragment_shader, sizeof(info_log), nullptr, info_log);
            std::println("Error: Fragment Shader: {}", info_log);
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
            return 0;
        }

        // Create shader program.
        GLint link_status;
        GLuint program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &link_status);
        if (link_status == GL_FALSE)
        {
            char info_log[512];
            glGetProgramInfoLog(program, sizeof(info_log), nullptr, info_log);
            std::println("Error: Shader Program: {}", info_log);
            glDeleteShader(vertex_shader);
            glDeleteShader(fragment_shader);
            glDeleteProgram(program);
            return 0;
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        return program;
    }
}

GLuint Shader::s_currenly_bound_id = 0;

Shader::Shader() = default;

Shader::~Shader()
{
    if (m_shader_program_id != 0)
    {
        glDeleteProgram(m_shader_program_id);
    }
}

void Shader::Create(const std::string& vshader_source, const std::string& fshader_source)
{
    Destroy();

    m_shader_program_id = LoadShaderProgram(vshader_source, fshader_source);
}

void Shader::Destroy()
{
    if (s_currenly_bound_id == m_shader_program_id)
    {
        glUseProgram(0);

        s_currenly_bound_id = 0;
    }

    glDeleteProgram(m_shader_program_id);

    m_shader_program_id = 0;

    m_uniform_location_cache.clear();
}

void Shader::Use()
{
    assert(m_shader_program_id != 0);

    if (s_currenly_bound_id == m_shader_program_id) { return; }

    glUseProgram(m_shader_program_id);

    s_currenly_bound_id = m_shader_program_id;
}

void Shader::SetUniform(const std::string& uniform_name, bool value)
{
    Use();

    glUniform1i(GetUniformLocation(uniform_name), static_cast<int>(value));
}

void Shader::SetUniform(const std::string& uniform_name, int value)
{
    Use();

    glUniform1i(GetUniformLocation(uniform_name), value);
}

void Shader::SetUniform(const std::string& uniform_name, float value)
{
    Use();

    glUniform1f(GetUniformLocation(uniform_name), value);
}

void Shader::SetUniform(const std::string& uniform_name, glm::vec2 value)
{
    Use();

    glUniform2f(GetUniformLocation(uniform_name), value.x, value.y);
}

void Shader::SetUniform(const std::string& uniform_name, glm::vec3 value)
{
    Use();

    glUniform3f(GetUniformLocation(uniform_name), value.x, value.y, value.z);
}

void Shader::SetUniform(const std::string& uniform_name, glm::vec4 value)
{
    Use();

    glUniform4f(GetUniformLocation(uniform_name), value.x, value.y, value.z, value.w);
}

void Shader::SetUniform(const std::string& uniform_name, const glm::mat2& value)
{
    Use();

    glUniformMatrix2fv(GetUniformLocation(uniform_name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetUniform(const std::string& uniform_name, const glm::mat3& value)
{
    Use();

    glUniformMatrix3fv(GetUniformLocation(uniform_name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetUniform(const std::string& uniform_name, const glm::mat4& value)
{
    Use();

    glUniformMatrix4fv(GetUniformLocation(uniform_name), 1, GL_FALSE, glm::value_ptr(value));
}

GLint Shader::GetUniformLocation(const std::string& uniform_name)
{
    if (auto iter = m_uniform_location_cache.find(uniform_name); iter != m_uniform_location_cache.end())
    {
        return iter->second;
    }

    GLint uniform_location = glGetUniformLocation(m_shader_program_id, uniform_name.c_str());

    m_uniform_location_cache.emplace(uniform_name, uniform_location);

    if (uniform_location == -1) std::println("OpenGL Error: Uniform location not found.");

    return uniform_location;
}

} // namespace nitrocraft::graphics
