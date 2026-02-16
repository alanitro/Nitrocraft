#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glad/gl.h>

class Graphics_Shader
{
public:
    Graphics_Shader();
    ~Graphics_Shader();
    Graphics_Shader(const Graphics_Shader&) = delete;
    Graphics_Shader& operator=(const Graphics_Shader&) = delete;

    void Create(const std::string& vshader_source, const std::string& fshader_source);
    void Destroy();

    void Use();

    void SetUniform(const std::string& uniform_name, bool value);
    void SetUniform(const std::string& uniform_name, int value);
    void SetUniform(const std::string& uniform_name, float value);
    void SetUniform(const std::string& uniform_name, glm::vec2 value);
    void SetUniform(const std::string& uniform_name, glm::vec3 value);
    void SetUniform(const std::string& uniform_name, glm::vec4 value);
    void SetUniform(const std::string& uniform_name, const glm::mat2& value);
    void SetUniform(const std::string& uniform_name, const glm::mat3& value);
    void SetUniform(const std::string& uniform_name, const glm::mat4& value);

private:
    GLuint m_ShaderProgramID = 0;

    std::unordered_map<std::string, GLint> m_UniformLocationCache;

private:
    GLint GetUniformLocation(const std::string& uniform_name);
};
