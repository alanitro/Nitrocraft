#pragma once

#include <optional>
#include <string>
#include <vector>

std::optional<std::string> IO_ReadFile(std::string_view filepath);

struct IO_Image
{
    std::vector<std::uint8_t> ImageData;
    int Width = 0;
    int Height = 0;
    int ChannelNumbers = 0;
};

std::optional<IO_Image> IO_ReadImage(std::string_view filepath, bool image_flip = false);
