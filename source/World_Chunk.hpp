#pragma once

#include <cstdint>
#include <memory>
#include "World_Definitions.hpp"
#include "World_Block.hpp"
#include "Utility_Array2D.hpp"
#include "Utility_Array3D.hpp"

using World_Chunk_BlockData  = Array3D<World_Block, World_CHUNK_X_SIZE, World_CHUNK_Y_SIZE, World_CHUNK_Z_SIZE, Array3DStoreOrder::YXZ>;
using World_Chunk_LightData  = Array3D<World_Light, World_CHUNK_X_SIZE, World_CHUNK_Y_SIZE, World_CHUNK_Z_SIZE, Array3DStoreOrder::YXZ>;
using World_Chunk_HeightData = Array2D<std::uint8_t, World_CHUNK_X_SIZE, World_CHUNK_Z_SIZE, Array2DStoreOrder::YX>;

struct World_Chunk_Payload
{
    World_Chunk_BlockData  Blocks;
    World_Chunk_LightData  Lights;
    World_Chunk_HeightData Heights;
};

struct World_Chunk
{
    const World_ChunkID   ID;

    bool            HasModified   = false;
    bool            NeighboursSet = false;

    World_Chunk*    NeighbourXNZ0 = nullptr;
    World_Chunk*    NeighbourXPZ0 = nullptr;
    World_Chunk*    NeighbourX0ZN = nullptr;
    World_Chunk*    NeighbourX0ZP = nullptr;
    World_Chunk*    NeighbourXNZN = nullptr;
    World_Chunk*    NeighbourXPZN = nullptr;
    World_Chunk*    NeighbourXNZP = nullptr;
    World_Chunk*    NeighbourXPZP = nullptr;

    std::unique_ptr<World_Chunk_Payload> Payload;

    World_Chunk(World_ChunkID id) : ID{ id } {}
};

World_Block World_Chunk_GetBlockAt(const World_Chunk* self, World_LocalXYZ local);
World_Light World_Chunk_GetSunlightAt(const World_Chunk* self, World_LocalXYZ local);
World_Light World_Chunk_GetPointlightAt(const World_Chunk* self, World_LocalXYZ local);

void World_Chunk_SetBlockAt(World_Chunk* self, World_LocalXYZ local, World_Block block);
void World_Chunk_SetSunlightAt(World_Chunk* self, World_LocalXYZ local, World_Light sunlight);
void World_Chunk_SetPointlightAt(World_Chunk* self, World_LocalXYZ local, World_Light pointlight);

std::array<World_Block, static_cast<std::size_t>(World_BlockNeighbour::COUNT)> World_Chunk_GetNeighbourBlocks(const World_Chunk* self, World_LocalXYZ local);
