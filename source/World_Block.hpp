#pragma once

#include <cstdint>
#include <string_view>
#include <glm/vec2.hpp>
#include "World_Definitions.hpp"

struct World_Block
{
    World_BlockID ID;

    bool operator==(World_Block rhs) const
    {
        return ID == rhs.ID;
    }

    bool operator!=(World_Block rhs) const
    {
        return ID != rhs.ID;
    }

    bool IsOpaque() const
    {
        return ID != World_BlockID::AIR && ID != World_BlockID::OAK_LEAVES;
    }

    bool IsTransparent() const
    {
        return ID == World_BlockID::AIR || ID == World_BlockID::OAK_LEAVES;
    }

    std::string_view GetBlockName();
};
