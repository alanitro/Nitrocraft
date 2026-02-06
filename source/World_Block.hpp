#pragma once

#include <cstdint>
#include <string_view>
#include <glm/vec2.hpp>
#include "World_Definitions.hpp"

struct World_Block
{
    World_BlockID ID;
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
    return self.ID != World_BlockID::AIR && self.ID != World_BlockID::OAK_LEAVES;
}

constexpr bool World_Block_IsTransparent(World_Block self)
{
    return self.ID == World_BlockID::AIR || self.ID == World_BlockID::OAK_LEAVES;
}

std::string_view World_Block_GetBlockName(World_BlockID block_id);
