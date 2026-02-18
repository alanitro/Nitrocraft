#pragma once

#include <cstdint>
#include <string_view>

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

enum class World_Block_CrossNeighbour
{
    XN, XP,
    YN, YP,
    ZN, ZP,

    Count = 6,
};

enum class World_Block_WholeNeighbour
{
    // Faces (6): one-axis offsets
    XnYoZo, XpYoZo,
    XoYnZo, XoYpZo,
    XoYoZn, XoYoZp,

    // Edges (12): two-axis offsets
    // XZ edges (y = 0)
    XnYoZn, XpYoZn,
    XnYoZp, XpYoZp,

    // YZ edges (x = 0)
    XoYnZn, XoYpZn,
    XoYnZp, XoYpZp,

    // XY edges (z = 0)
    XnYnZo, XpYnZo, 
    XnYpZo, XpYpZo,

    // Corners (8): three-axis offsets
    XnYnZn, XpYnZn,
    XnYpZn, XpYpZn,
    XnYnZp, XpYnZp,
    XnYpZp, XpYpZp,

    Count,
};

enum class World_Block_Face
{
    XN, XP,
    YN, YP,
    ZN, ZP,

    COUNT = 6,
};

struct World_Block
{
    World_Block_ID ID;

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
        return ID != World_Block_ID::AIR && ID != World_Block_ID::OAK_LEAVES;
    }

    bool IsTransparent() const
    {
        return ID == World_Block_ID::AIR || ID == World_Block_ID::OAK_LEAVES;
    }

    std::string_view GetBlockName() const;
};
