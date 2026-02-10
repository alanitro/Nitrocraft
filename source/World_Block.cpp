#include "World_Block.hpp"

namespace
{
    constexpr std::string_view BLOCK_NAMES[static_cast<std::size_t>(World_BlockID::COUNT)]
    {
        "Air",
        "Stone",
        "Bedrock",
        "Dirt",
        "Grass",
        "Sand",
        "Snow",
        "Brick",
        "Glowstone",
        "Oak",
        "Oak Leaves",
        "Oak Wood",
    };
}

std::string_view World_Block::GetBlockName()
{
    return BLOCK_NAMES[static_cast<std::size_t>(ID)];
}
