#include "Utility_File.hpp"

#include <sstream>
#include <fstream>
#include <print>
#include <stb/stb_image.h>

std::optional<std::string> File::ReadFile(std::string_view filepath)
{
    std::ifstream file;

    file.open(std::string(filepath));

    if (file.is_open() == false) return std::nullopt;

    std::stringstream buffer;

    buffer << file.rdbuf();

    return buffer.str();
}

std::optional<File::Image> File::ReadImage(std::string_view filepath, bool image_flip)
{
    stbi_set_flip_vertically_on_load(image_flip);

    int width, height, channel_numbers;

    std::uint8_t* image_ptr = stbi_load(std::string(filepath).c_str(), &width, &height, &channel_numbers, 0);

    if (image_ptr == nullptr) return std::nullopt;

    std::vector<std::uint8_t> image_data(image_ptr, image_ptr + (width * height * channel_numbers));

    stbi_image_free(image_ptr);

    return File::Image(image_data, width, height, channel_numbers);
}
