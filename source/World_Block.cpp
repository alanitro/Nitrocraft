#include "World_Block.hpp"

namespace nitrocraft::world
{

namespace
{
    constexpr std::string_view BLOCK_NAMES[static_cast<std::size_t>(BlockID::COUNT)]
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

std::string_view Block::GetBlockName() const
{
    return BLOCK_NAMES[static_cast<std::size_t>(id)];
}

} // namespace nitrocraft::world
