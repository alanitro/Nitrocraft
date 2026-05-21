#include "Utility_IO.hpp"

#include <sstream>
#include <fstream>
#include <print>
#include <stb/stb_image.h>

namespace nitrocraft::utility
{

std::optional<std::string> ReadFile(std::string_view filepath)
{
    std::ifstream file;

    file.open(std::string(filepath));

    if (file.is_open() == false)
    {
        return std::nullopt;
    }

    std::stringstream buffer;

    buffer << file.rdbuf();

    return buffer.str();
}

std::optional<ImageData> ReadImage(std::string_view filepath, bool image_flip)
{
    stbi_set_flip_vertically_on_load(image_flip);

    int width, height, channel_numbers;

    std::uint8_t* raw_image_ptr = stbi_load(std::string(filepath).c_str(), &width, &height, &channel_numbers, 0);

    if (raw_image_ptr == nullptr)
    {
        return std::nullopt;
    }

    std::unique_ptr<std::uint8_t, decltype(&stbi_image_free)> image_ptr(
        raw_image_ptr,
        stbi_image_free
    );

    std::size_t image_size = static_cast<std::size_t>(width * height * channel_numbers);

    std::vector<std::uint8_t> image_data(image_ptr.get(), image_ptr.get() + image_size);

    return ImageData(image_data, width, height, channel_numbers);
}

} // namespace nitrocraft::utility
