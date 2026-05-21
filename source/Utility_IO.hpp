#pragma once

#include <optional>
#include <string>
#include <vector>

namespace nitrocraft::utility
{

std::optional<std::string> ReadFile(std::string_view filepath);

struct ImageData
{
    std::vector<std::uint8_t> data;
    int width = 0;
    int height = 0;
    int channel_numbers = 0;
};

std::optional<ImageData> ReadImage(std::string_view filepath, bool image_flip = false);

} // namespace nitrocraft::utility
