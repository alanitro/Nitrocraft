#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glad/gl.h>

namespace nitrocraft::graphics
{

class Shader
{
public:
    Shader();
    ~Shader();
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

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
    static GLuint s_currenly_bound_id;

    GLuint m_shader_program_id = 0;

    std::unordered_map<std::string, GLint> m_uniform_location_cache;

private:
    GLint GetUniformLocation(const std::string& uniform_name);
};

} // namespace nitrocraft::graphics
