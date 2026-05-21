#pragma once

#include <cstdint>
#include <string_view>

namespace nitrocraft::world
{

enum class BlockID : std::uint8_t
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

enum class BlockCrossNeighbour
{
    XN, XP,
    YN, YP,
    ZN, ZP,

    Count = 6,
};

enum class BlockWholeNeighbour
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

enum class BlockFace
{
    XN, XP,
    YN, YP,
    ZN, ZP,

    COUNT = 6,
};

struct Block
{
    BlockID id;

    bool operator==(Block rhs) const
    {
        return id == rhs.id;
    }

    bool operator!=(Block rhs) const
    {
        return id != rhs.id;
    }

    bool IsOpaque() const
    {
        return id != BlockID::AIR && id != BlockID::OAK_LEAVES;
    }

    bool IsTransparent() const
    {
        return id == BlockID::AIR || id == BlockID::OAK_LEAVES;
    }

    std::string_view GetBlockName() const;
};

} // namespace nitrocraft::world
