#include "Graphics_Shader.hpp"

#include <cassert>
#include <print>
#include <glm/gtc/type_ptr.hpp>

namespace
{
    GLuint Graphics_Shader_CurrentlyBoundID = 0;

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

Graphics_Shader::Graphics_Shader() = default;

Graphics_Shader::~Graphics_Shader()
{
    if (m_ShaderProgramID != 0)
    {
        glDeleteProgram(m_ShaderProgramID);
    }
}

void Graphics_Shader::Create(const std::string& vshader_source, const std::string& fshader_source)
{
    Destroy();

    m_ShaderProgramID = LoadShaderProgram(vshader_source, fshader_source);
}

void Graphics_Shader::Destroy()
{
    if (Graphics_Shader_CurrentlyBoundID == m_ShaderProgramID)
    {
        glUseProgram(0);

        Graphics_Shader_CurrentlyBoundID = 0;
    }

    glDeleteProgram(m_ShaderProgramID);

    m_ShaderProgramID = 0;

    m_UniformLocationCache.clear();
}

void Graphics_Shader::Use()
{
    assert(m_ShaderProgramID != 0);

    if (Graphics_Shader_CurrentlyBoundID == m_ShaderProgramID) { return; }

    glUseProgram(m_ShaderProgramID);

    Graphics_Shader_CurrentlyBoundID = m_ShaderProgramID;
}

void Graphics_Shader::SetUniform(const std::string& uniform_name, bool value)
{
    Use();

    glUniform1i(GetUniformLocation(uniform_name), static_cast<int>(value));
}

void Graphics_Shader::SetUniform(const std::string& uniform_name, int value)
{
    Use();

    glUniform1i(GetUniformLocation(uniform_name), value);
}

void Graphics_Shader::SetUniform(const std::string& uniform_name, float value)
{
    Use();

    glUniform1f(GetUniformLocation(uniform_name), value);
}

void Graphics_Shader::SetUniform(const std::string& uniform_name, glm::vec2 value)
{
    Use();

    glUniform2f(GetUniformLocation(uniform_name), value.x, value.y);
}

void Graphics_Shader::SetUniform(const std::string& uniform_name, glm::vec3 value)
{
    Use();

    glUniform3f(GetUniformLocation(uniform_name), value.x, value.y, value.z);
}

void Graphics_Shader::SetUniform(const std::string& uniform_name, glm::vec4 value)
{
    Use();

    glUniform4f(GetUniformLocation(uniform_name), value.x, value.y, value.z, value.w);
}

void Graphics_Shader::SetUniform(const std::string& uniform_name, const glm::mat2& value)
{
    Use();

    glUniformMatrix2fv(GetUniformLocation(uniform_name), 1, GL_FALSE, glm::value_ptr(value));
}

void Graphics_Shader::SetUniform(const std::string& uniform_name, const glm::mat3& value)
{
    Use();

    glUniformMatrix3fv(GetUniformLocation(uniform_name), 1, GL_FALSE, glm::value_ptr(value));
}

void Graphics_Shader::SetUniform(const std::string& uniform_name, const glm::mat4& value)
{
    Use();

    glUniformMatrix4fv(GetUniformLocation(uniform_name), 1, GL_FALSE, glm::value_ptr(value));
}

GLint Graphics_Shader::GetUniformLocation(const std::string& uniform_name)
{
    if (auto iter = m_UniformLocationCache.find(uniform_name); iter != m_UniformLocationCache.end())
    {
        return iter->second;
    }

    GLint uniform_location = glGetUniformLocation(m_ShaderProgramID, uniform_name.c_str());

    m_UniformLocationCache.emplace(uniform_name, uniform_location);

    if (uniform_location == -1) std::println("OpenGL Error: Uniform location not found.");

    return uniform_location;
}
