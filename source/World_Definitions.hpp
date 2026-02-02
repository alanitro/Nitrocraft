#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// Coordinates
using WorldPosition = glm::ivec3; // World relative position
using ChunkPosition = glm::ivec3; // Chunk relative position
using ChunkID  = glm::ivec3; // Center chunk ID = (0,0,0), positive +1x chunk to center chunk = (1,0,0)

// World Constants
constexpr int WORLD_HEIGHT    = 256;
constexpr int WORLD_SEA_LEVEL = 64;
constexpr int WORLD_RENDER_DISTANCE = 6; // Render radius in Chunk unit

// Loading Constants
constexpr int WORLD_LOADING_RADIUS = WORLD_RENDER_DISTANCE + 3;
constexpr int WORLD_LOADING_DIAMETER = WORLD_LOADING_RADIUS * 2 + 1;
constexpr int WORLD_LOADING_AREA = WORLD_LOADING_DIAMETER * WORLD_LOADING_DIAMETER;

// Chunk Constants
constexpr int WORLD_CHUNK_X_SIZE = 16;
constexpr int WORLD_CHUNK_Y_SIZE = WORLD_HEIGHT;
constexpr int WORLD_CHUNK_Z_SIZE = 16;
constexpr int WORLD_CHUNK_AREA   = WORLD_CHUNK_X_SIZE * WORLD_CHUNK_Z_SIZE;
constexpr int WORLD_CHUNK_VOLUME = WORLD_CHUNK_X_SIZE * WORLD_CHUNK_Y_SIZE * WORLD_CHUNK_Z_SIZE;

constexpr ChunkID FromWorldPositionToChunkID(WorldPosition position)
{
    int& x = position.x;
    int& y = position.y;
    int& z = position.z;
    (void)y;

    constexpr int sx = WORLD_CHUNK_X_SIZE;
    constexpr int sz = WORLD_CHUNK_Z_SIZE;

    return ChunkID{
        (((x % sx >= 0) ? x : (x - sx)) / sx),
        0,
        (((z % sz >= 0) ? z : (z - sz)) / sz)
    };
}

constexpr WorldPosition FromWorldPositionToChunkOffset(WorldPosition position)
{
    return FromWorldPositionToChunkID(position) * WorldPosition{ WORLD_CHUNK_X_SIZE, WORLD_CHUNK_Y_SIZE, WORLD_CHUNK_Z_SIZE };
}

constexpr ChunkPosition FromWorldPositionToChunkPosition(WorldPosition position)
{
    return position - FromWorldPositionToChunkOffset(position);
}

constexpr WorldPosition FromChunkIDToChunkOffset(ChunkID chunk_id)
{
    return chunk_id * WorldPosition{ WORLD_CHUNK_X_SIZE, WORLD_CHUNK_Y_SIZE, WORLD_CHUNK_Z_SIZE };
}
