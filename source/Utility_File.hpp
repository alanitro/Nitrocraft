#pragma once

#include <optional>
#include <string>
#include <vector>

namespace File
{
    std::optional<std::string> ReadFile(std::string_view filepath);

    struct Image
    {
        std::vector<std::uint8_t> ImageData;
        int Width = 0;
        int Height = 0;
        int ChannelNumbers = 0;
    };

    std::optional<Image> ReadImage(std::string_view filepath, bool image_flip = false);
}
