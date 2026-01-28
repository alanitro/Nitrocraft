#pragma once

#include <glad/gl.h>
#include <optional>
#include <string>
#include <string_view>

namespace ResourceManager
{
    std::optional<std::string> LoadFile(std::string_view filepath);

    std::optional<GLuint> LoadShaderProgram(std::string_view shader_name);

    std::optional<GLuint> LoadTexture(std::string_view texture_name);
}
