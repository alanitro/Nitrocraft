#pragma once

#include <cstdint>
#include <cstddef>
#include "World_Definitions.hpp"
#include "World_Block.hpp"
#include "Utility_Array2D.hpp"
#include "Utility_Array3D.hpp"

using ChunkBlockData  = Array3D<Block, WORLD_CHUNK_X_SIZE, WORLD_CHUNK_Y_SIZE, WORLD_CHUNK_Z_SIZE, Array3DStoreOrder::YXZ>;
using ChunkHeightData = Array2D<std::uint8_t, WORLD_CHUNK_X_SIZE, WORLD_CHUNK_Z_SIZE>;

struct Chunk
{
    const ChunkID ID;

    bool   NeighboursSet = false;

    Chunk* NeighbourXNZ0 = nullptr;
    Chunk* NeighbourXPZ0 = nullptr;
    Chunk* NeighbourX0ZN = nullptr;
    Chunk* NeighbourX0ZP = nullptr;
    Chunk* NeighbourXNZN = nullptr;
    Chunk* NeighbourXPZN = nullptr;
    Chunk* NeighbourXNZP = nullptr;
    Chunk* NeighbourXPZP = nullptr;

    ChunkBlockData  BlockData;
    ChunkHeightData HeightData;

    explicit Chunk(ChunkID id) : ID{ id } {}

    ChunkID  GetID() const { return ID; }
    WorldPosition GetOffset() const { return ID * WorldPosition{ WORLD_CHUNK_X_SIZE, WORLD_CHUNK_Y_SIZE, WORLD_CHUNK_Z_SIZE }; }

    Block GetBlockAt(int x, int y, int z) const;
    Block GetBlockAt(ChunkPosition position)   const;

    std::array<Block, (int)BlockNeighbour::COUNT> GetNeighbourBlocksAt(ChunkPosition position) const;
};
