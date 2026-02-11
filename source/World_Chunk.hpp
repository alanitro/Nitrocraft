#pragma once

#include <cstdint>
#include <memory>
#include <array>
#include "World_Coordinate.hpp"
#include "World_Block.hpp"
#include "World_Light.hpp"
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

enum class World_Chunk_Neighbour
{
    XNZ0,
    XPZ0,
    X0ZN,
    X0ZP,
    XNZN,
    XPZN,
    XNZP,
    XPZP,

    COUNT,
};

enum class World_Chunk_Stage
{
    Empty,

    GenerationRequested,
    Generating,
    GenerationComplete,

    LightingRequested,
    Lighting,
    LightingComplete,

    MeshingRequested,
    Meshing,
    MeshingComplete,
    
    COUNT,
};

struct World_Chunk
{
    const World_ChunkID   ID;

    World_Chunk_Stage Stage = World_Chunk_Stage::Empty;

    bool HasModified   = false;
    bool NeighboursSet = false;

    std::array<World_Chunk*, (std::size_t)World_Chunk_Neighbour::COUNT> Neighbours{};

    std::unique_ptr<World_Chunk_Payload> Payload;

    explicit World_Chunk(World_ChunkID id) : ID{ id } {}

    World_Block GetBlockAt(World_LocalXYZ local) const;
    World_Light GetLightAt(World_LocalXYZ local) const;
    World_Light GetSunlightAt(World_LocalXYZ local) const;
    World_Light GetPointlightAt(World_LocalXYZ local) const;

    void SetBlockAt(World_LocalXYZ local, World_Block block);
    void SetLightAt(World_LocalXYZ local, World_Light sunlight, World_Light pointlight);
    void SetSunlightAt(World_LocalXYZ local, World_Light sunlight);
    void SetPointlightAt(World_LocalXYZ local, World_Light pointlight);

    int  GetHeightAt(int local_x, int local_z);
    void SetHeightAt(int local_x, int local_z, std::uint8_t height);

    std::array<World_Block, static_cast<std::size_t>(World_Block_Neighbour::COUNT)> GetNeighbourBlocksAt(World_LocalXYZ local) const;
    std::array<World_Light, static_cast<std::size_t>(World_Block_Neighbour::COUNT)> GetNeighbourLightsAt(World_LocalXYZ local) const;
};
