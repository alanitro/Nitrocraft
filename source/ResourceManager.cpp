#include "ResourceManager.hpp"

#include <format>
#include <sstream>
#include <fstream>
#include <iostream>
#include <print>
#include "stb/stb_image.h"

std::optional<std::string> ResourceManager::LoadFile(std::string_view filepath)
{
    std::ifstream file;
    file.open(std::string(filepath));

    if (file.is_open() == false)
    {
        std::println("Error: File {} does not exist", filepath);
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

std::optional<GLuint> ResourceManager::LoadShaderProgram(std::string_view shader_name)
{
    std::string vertex_shader_path = std::format("{}{}{}", "./resource/shader/", shader_name, ".vert.glsl");
    std::string fragment_shader_path = std::format("{}{}{}", "./resource/shader/", shader_name, ".frag.glsl");

    // Create vertex shader
    auto vertex_shader_source_opt = LoadFile(vertex_shader_path);
    if (vertex_shader_source_opt.has_value() == false) return std::nullopt;
    const char* vertex_shader_source = vertex_shader_source_opt.value().c_str();

    GLint compile_status;
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
    if (compile_status == GL_FALSE)
    {
        char info_log[512];
        glGetShaderInfoLog(vertex_shader, sizeof(info_log), nullptr, info_log);
        std::println(std::cerr, "ERROR:SHADER:VERTEX: {}", info_log);
        return std::nullopt;
    }

    // Create fragment shader
    auto fragment_shader_source_opt = LoadFile(fragment_shader_path);
    if (fragment_shader_source_opt.has_value() == false) return std::nullopt;
    const char* fragment_shader_source = fragment_shader_source_opt.value().c_str();

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_status);
    if (compile_status == GL_FALSE)
    {
        char info_log[512];
        glGetShaderInfoLog(fragment_shader, sizeof(info_log), nullptr, info_log);
        std::println(std::cerr, "ERROR:SHADER:FRAGMENT: {}", info_log);
        return std::nullopt;
    }

    // Create shader program
    GLint link_status;
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (link_status == GL_FALSE)
    {
        char info_log[512];
        glGetProgramInfoLog(fragment_shader, sizeof(info_log), nullptr, info_log);
        std::println(std::cerr, "ERROR: SHADER PROGRAM: {}", info_log);
        return std::nullopt;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

std::optional<GLuint> ResourceManager::LoadTexture(std::string_view texture_name)
{
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    stbi_set_flip_vertically_on_load(true);
    int width, height, channels;
    std::uint8_t* data = stbi_load(std::format("./resource/texture/{}.png", texture_name).c_str(), &width, &height, &channels, 0);
    GLint format = 0;
    if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;
    else return std::nullopt;

    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else return std::nullopt;

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}
