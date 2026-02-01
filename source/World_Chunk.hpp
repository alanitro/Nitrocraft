#pragma once

#include <cstdint>
#include <cstddef>
#include "World_Definitions.hpp"
#include "World_Block.hpp"
#include "Utility_Array3D.hpp"

struct Chunk
{
public:
    bool   Modified     = false;
    bool   NeighboursSet = false;

    Chunk* NeighbourXNZ0 = nullptr;
    Chunk* NeighbourXPZ0 = nullptr;
    Chunk* NeighbourX0ZN = nullptr;
    Chunk* NeighbourX0ZP = nullptr;
    Chunk* NeighbourXNZN = nullptr;
    Chunk* NeighbourXPZN = nullptr;
    Chunk* NeighbourXNZP = nullptr;
    Chunk* NeighbourXPZP = nullptr;

    Array3D<Block, WORLD_CHUNK_X_SIZE, WORLD_CHUNK_Y_SIZE, WORLD_CHUNK_Z_SIZE, Array3DStoreOrder::YXZ> Blocks;

    Block GetBlockAt(int x, int y, int z) const;

    Block GetBlockAt(ChunkXYZ position) const;

    std::array<Block, (int)BlockNeighbour::COUNT> GetNeighbourBlocksAt(ChunkXYZ position) const;
};
