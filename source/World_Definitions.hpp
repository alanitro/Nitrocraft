#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// Coordinates
using WorldXYZ = glm::ivec3; // World relative position
using ChunkXYZ = glm::ivec3; // Chunk relative position
using ChunkID  = glm::ivec3; // Center chunk ID = (0,0,0), positive +1x chunk to center chunk = (1,0,0)

// World Constants
constexpr int WORLD_HEIGHT    = 256;
constexpr int WORLD_SEA_LEVEL = 80;
constexpr int WORLD_RENDER_DISTANCE = 6; // Render radius in Chunk unit

// Chunk Constants
constexpr int WORLD_CHUNK_X_SIZE = 32;
constexpr int WORLD_CHUNK_Y_SIZE = WORLD_HEIGHT;
constexpr int WORLD_CHUNK_Z_SIZE = 32;
constexpr int WORLD_CHUNK_AREA   = WORLD_CHUNK_X_SIZE * WORLD_CHUNK_Z_SIZE;
constexpr int WORLD_CHUNK_VOLUME = WORLD_CHUNK_X_SIZE * WORLD_CHUNK_Y_SIZE * WORLD_CHUNK_Z_SIZE;

constexpr ChunkID ToChunkID(WorldXYZ position)
{
    int& x = position.x;
    int& y = position.y;
    int& z = position.z;
    (void)y;

    constexpr int sx = WORLD_CHUNK_X_SIZE;
    constexpr int sz = WORLD_CHUNK_Z_SIZE;

    return ChunkID
    (
        (((x % sx >= 0) ? x : (x - sx)) / sx),
        0,
        (((z % sz >= 0) ? z : (z - sz)) / sz)
    );
}

constexpr WorldXYZ ToChunkOffset(WorldXYZ position)
{
    return ToChunkID(position) * WorldXYZ(WORLD_CHUNK_X_SIZE, 0, WORLD_CHUNK_Z_SIZE);
}

constexpr ChunkXYZ ToChunkXYZ(WorldXYZ position)
{
    return position - ToChunkOffset(position);
}

// Loading Constants
constexpr int WORLD_LOADING_RADIUS   = WORLD_RENDER_DISTANCE + 3;
constexpr int WORLD_LOADING_DIAMETER = WORLD_LOADING_RADIUS * 2 + 1;
constexpr int WORLD_LOADING_AREA     = WORLD_LOADING_DIAMETER * WORLD_LOADING_DIAMETER;
