#pragma once

#include <cstdint>
#include <string_view>
#include <glm/vec2.hpp>

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

enum class BlockNeighbour
{
    XN, XP,
    YN, YP,
    ZN, ZP,

    COUNT = 6,
};

struct Block
{
    BlockID ID = BlockID::AIR;

    Block() = default;
    Block(BlockID id) : ID{ id } {}

    bool operator==(Block block) const { return ID == block.ID; }
    bool operator!=(Block block) const { return ID != block.ID; }
    bool IsOpaque()      const { return ID != BlockID::AIR && ID != BlockID::OAK_LEAVES; }
    bool IsTransparent() const { return ID == BlockID::AIR || ID == BlockID::OAK_LEAVES; }

    static std::string_view GetBlockName(BlockID id);
};
