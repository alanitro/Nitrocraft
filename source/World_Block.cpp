#include "World_Block.hpp"

#include <string_view>

namespace
{
    constexpr std::string_view BlockNames[static_cast<int>(BlockID::COUNT)]
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

std::string_view Block::GetBlockName(BlockID id)
{
    return BlockNames[static_cast<int>(id)];
}
