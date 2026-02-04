#pragma once

#include <cstdint>
#include <string_view>
#include <glm/vec2.hpp>

enum class World_Block_ID : std::uint8_t
{
    AIR,
    STONE,
    BEDROCK,
    DIRT,
    GRASS,
    SAND,
    SNOW,
    BRICK,
    GLOWSTONE,
    OAK,
    OAK_LEAVES,
    OAK_WOOD,

    COUNT,
};

enum class World_Block_Neighbour
{
    XN, XP,
    YN, YP,
    ZN, ZP,

    COUNT = 6,
};

enum class World_Block_Face
{
    XN, XP,
    YN, YP,
    ZN, ZP,

    COUNT = 6,
};

enum class World_Block_FaceBit : std::uint32_t
{
    XN = (1u << static_cast<std::uint32_t>(World_Block_Face::XN)),
    XP = (1u << static_cast<std::uint32_t>(World_Block_Face::XP)),
    YN = (1u << static_cast<std::uint32_t>(World_Block_Face::YN)),
    YP = (1u << static_cast<std::uint32_t>(World_Block_Face::YP)),
    ZN = (1u << static_cast<std::uint32_t>(World_Block_Face::ZN)),
    ZP = (1u << static_cast<std::uint32_t>(World_Block_Face::ZP)),

    COUNT = 6,
};

struct World_Block
{
    World_Block_ID ID = World_Block_ID::AIR;
};

constexpr bool World_Block_IsEqual(World_Block block1, World_Block  block2)
{
    return block1.ID == block2.ID;
}

constexpr bool World_Block_IsNotEqual(World_Block block1, World_Block  block2)
{
    return block1.ID != block2.ID;
}

constexpr bool World_Block_IsOpaque(World_Block self)
{
    return self.ID != World_Block_ID::AIR && self.ID != World_Block_ID::OAK_LEAVES;
}

constexpr bool World_Block_IsTransparent(World_Block self)
{
    return self.ID == World_Block_ID::AIR || self.ID == World_Block_ID::OAK_LEAVES;
}

std::string_view World_Block_GetBlockName(World_Block_ID block_id);
