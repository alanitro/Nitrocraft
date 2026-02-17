#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// Coordinates (Right Handed Coordinate)
using World_Position  = glm::vec3;  // World relative position
using World_GlobalXYZ = glm::ivec3; // World relative position
using World_LocalXYZ  = glm::ivec3; // Chunk relative position
using World_Chunk_ID  = glm::ivec3; // Center chunk ID = (0,0,0), positive +1x chunk to center chunk = (1,0,0)

// World Constants
constexpr int World_HEIGHT          = 256;
constexpr int World_SEA_LEVEL       = 64;

// Chunk Constants
constexpr int World_CHUNK_X_SIZE = 16;
constexpr int World_CHUNK_Y_SIZE = World_HEIGHT;
constexpr int World_CHUNK_Z_SIZE = 16;
constexpr int World_CHUNK_AREA   = World_CHUNK_X_SIZE * World_CHUNK_Z_SIZE;
constexpr int World_CHUNK_VOLUME = World_CHUNK_X_SIZE * World_CHUNK_Y_SIZE * World_CHUNK_Z_SIZE;

constexpr World_Chunk_ID World_FromGlobalToChunkID(World_GlobalXYZ position)
{
    int& x = position.x;
    //int& y = position.y;
    int& z = position.z;

    constexpr int sx = World_CHUNK_X_SIZE;
    //constexpr int sy = World_CHUNK_Y_SIZE;
    constexpr int sz = World_CHUNK_Z_SIZE;

    return World_Chunk_ID{
        (((x % sx >= 0) ? x : (x - sx)) / sx),
        //(((y % sy >= 0) ? y : (y - sy)) / sy),
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

constexpr World_GlobalXYZ World_FromChunkIDToChunkOffset(World_Chunk_ID chunk_id)
{
    return chunk_id * World_GlobalXYZ{ World_CHUNK_X_SIZE, World_CHUNK_Y_SIZE, World_CHUNK_Z_SIZE };
}
