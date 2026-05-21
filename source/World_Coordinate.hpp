#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace nitrocraft::world
{

// Coordinates (Right Handed Coordinate)
using Position  = glm::vec3;  // World relative position
using GlobalXYZ = glm::ivec3; // World relative position
using LocalXYZ  = glm::ivec3; // Chunk relative position
using ChunkID   = glm::ivec3; // Center chunk ID = (0,0,0), positive +1x chunk to center chunk = (1,0,0)

// World Constants
constexpr int HEIGHT    = 256;
constexpr int SEA_LEVEL = 64;

// Chunk Constants
constexpr int CHUNK_X_SIZE = 16;
constexpr int CHUNK_Y_SIZE = HEIGHT;
constexpr int CHUNK_Z_SIZE = 16;
constexpr int CHUNK_AREA   = CHUNK_X_SIZE * CHUNK_Z_SIZE;
constexpr int CHUNK_VOLUME = CHUNK_X_SIZE * CHUNK_Y_SIZE * CHUNK_Z_SIZE;

constexpr ChunkID FromGlobalToChunkID(GlobalXYZ position)
{
    int& x = position.x;
    //int& y = position.y;
    int& z = position.z;

    constexpr int sx = CHUNK_X_SIZE;
    //constexpr int sy = CHUNK_Y_SIZE;
    constexpr int sz = CHUNK_Z_SIZE;

    return ChunkID(
        (((x % sx >= 0) ? x : (x - sx)) / sx),
        //(((y % sy >= 0) ? y : (y - sy)) / sy),
        0,
        (((z % sz >= 0) ? z : (z - sz)) / sz)
    );
}

constexpr GlobalXYZ FromGlobalToChunkOffset(GlobalXYZ position)
{
    return FromGlobalToChunkID(position) * GlobalXYZ{ CHUNK_X_SIZE, CHUNK_Y_SIZE, CHUNK_Z_SIZE };
}

constexpr LocalXYZ FromGlobalToLocal(GlobalXYZ position)
{
    return position - FromGlobalToChunkOffset(position);
}

constexpr GlobalXYZ FromChunkIDToChunkOffset(ChunkID chunk_id)
{
    return chunk_id * GlobalXYZ{ CHUNK_X_SIZE, CHUNK_Y_SIZE, CHUNK_Z_SIZE };
}

} // namespace nitrocraft::world
