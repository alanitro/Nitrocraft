#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// Block constants
enum class World_BlockID : std::uint8_t
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

enum class World_BlockNeighbour
{
    XN, XP,
    YN, YP,
    ZN, ZP,

    COUNT = 6,
};

enum class World_BlockFace
{
    XN, XP,
    YN, YP,
    ZN, ZP,

    COUNT = 6,
};

enum class World_BlockFaceBit : std::uint32_t
{
    XN = (1u << static_cast<std::uint32_t>(World_BlockFace::XN)),
    XP = (1u << static_cast<std::uint32_t>(World_BlockFace::XP)),
    YN = (1u << static_cast<std::uint32_t>(World_BlockFace::YN)),
    YP = (1u << static_cast<std::uint32_t>(World_BlockFace::YP)),
    ZN = (1u << static_cast<std::uint32_t>(World_BlockFace::ZN)),
    ZP = (1u << static_cast<std::uint32_t>(World_BlockFace::ZP)),
};

// Coordinates (Right handed coordinate)
using World_Position  = glm::vec3;  // World relative position
using World_GlobalXYZ = glm::ivec3; // World relative position
using World_LocalXYZ  = glm::ivec3; // Chunk relative position
using World_ChunkID   = glm::ivec3; // Center chunk ID = (0,0,0), positive +1x chunk to center chunk = (1,0,0)

// World Constants
constexpr int World_HEIGHT          = 256;
constexpr int World_SEA_LEVEL       = 64;
constexpr int World_RENDER_DISTANCE = 10; // Render radius in Chunk unit

// Loading Constants
constexpr int World_LOADING_RADIUS      = World_RENDER_DISTANCE + 3;
constexpr int World_LOADING_DIAMETER    = World_LOADING_RADIUS * 2 + 1;
constexpr int World_LOADING_AREA        = World_LOADING_DIAMETER * World_LOADING_DIAMETER;

// Chunk Constants
constexpr int World_CHUNK_X_SIZE = 16;
constexpr int World_CHUNK_Y_SIZE = World_HEIGHT;
constexpr int World_CHUNK_Z_SIZE = 16;
constexpr int World_CHUNK_AREA   = World_CHUNK_X_SIZE * World_CHUNK_Z_SIZE;
constexpr int World_CHUNK_VOLUME = World_CHUNK_X_SIZE * World_CHUNK_Y_SIZE * World_CHUNK_Z_SIZE;

constexpr World_ChunkID World_FromGlobalToChunkID(World_GlobalXYZ position)
{
    int& x = position.x;
    int& y = position.y;
    int& z = position.z;
    (void)y;

    constexpr int sx = World_CHUNK_X_SIZE;
    constexpr int sz = World_CHUNK_Z_SIZE;

    return World_ChunkID{
        (((x % sx >= 0) ? x : (x - sx)) / sx),
        0,
        (((z % sz >= 0) ? z : (z - sz)) / sz)
    };
}

constexpr World_GlobalXYZ World_FromGlobalToChunkOffset(World_GlobalXYZ position)
{
    return World_FromGlobalToChunkID(position) * World_GlobalXYZ{ World_CHUNK_X_SIZE, World_CHUNK_Y_SIZE, World_CHUNK_Z_SIZE };
}

constexpr World_LocalXYZ World_FromGlobalToLocal(World_GlobalXYZ position)
{
    return position - World_FromGlobalToChunkOffset(position);
}

constexpr World_GlobalXYZ World_FromChunkIDToChunkOffset(World_ChunkID chunk_id)
{
    return chunk_id * World_GlobalXYZ{ World_CHUNK_X_SIZE, World_CHUNK_Y_SIZE, World_CHUNK_Z_SIZE };
}

// Lighting Constants
using World_Light = std::uint8_t; // ppppssss p = Pointlight, s = Sunlight 

constexpr World_Light World_LIGHT_LEVEL_00 = 0x00;
constexpr World_Light World_LIGHT_LEVEL_01 = 0x01;
constexpr World_Light World_LIGHT_LEVEL_02 = 0x02;
constexpr World_Light World_LIGHT_LEVEL_03 = 0x03;
constexpr World_Light World_LIGHT_LEVEL_04 = 0x04;
constexpr World_Light World_LIGHT_LEVEL_05 = 0x05;
constexpr World_Light World_LIGHT_LEVEL_06 = 0x06;
constexpr World_Light World_LIGHT_LEVEL_07 = 0x07;
constexpr World_Light World_LIGHT_LEVEL_08 = 0x08;
constexpr World_Light World_LIGHT_LEVEL_09 = 0x09;
constexpr World_Light World_LIGHT_LEVEL_10 = 0x0A;
constexpr World_Light World_LIGHT_LEVEL_11 = 0x0B;
constexpr World_Light World_LIGHT_LEVEL_12 = 0x0C;
constexpr World_Light World_LIGHT_LEVEL_13 = 0x0D;
constexpr World_Light World_LIGHT_LEVEL_14 = 0x0E;
constexpr World_Light World_LIGHT_LEVEL_15 = 0x0F;
constexpr World_Light World_LIGHT_LEVEL_MIN = World_LIGHT_LEVEL_00;
constexpr World_Light World_LIGHT_LEVEL_MAX = World_LIGHT_LEVEL_15;
constexpr World_Light World_LIGHT_LEVEL_SUN = World_LIGHT_LEVEL_MAX;
constexpr World_Light World_LIGHT_LEVEL_POINT = World_LIGHT_LEVEL_MAX;

constexpr World_Light World_ExtractSunlight(World_Light light)
{
    return (light >> 0) & 0x0F;
}

constexpr World_Light World_ExtractPointlight(World_Light light)
{
    return (light >> 4) & 0x0F;
}
