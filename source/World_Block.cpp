#include "World_Block.hpp"

namespace
{
    constexpr std::string_view BLOCK_NAMES[static_cast<std::size_t>(World_Block_ID::COUNT)]
    {
        "Air",
        "Stone",
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

std::string_view World_Block_GetBlockName(World_Block_ID block_id)
{
    return BLOCK_NAMES[static_cast<std::size_t>(block_id)];
}
