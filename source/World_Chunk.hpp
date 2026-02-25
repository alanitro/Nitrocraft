#pragma once

#include <cstdint>
#include <bitset>
#include <memory>
#include <array>
#include <vector>
#include "World_Coordinate.hpp"
#include "World_Block.hpp"
#include "World_Light.hpp"
#include "Graphics_Mesh.hpp"
#include "Utility_Array2D.hpp"
#include "Utility_Array3D.hpp"

using World_Chunk_BlockData  = Array3D<World_Block, World_CHUNK_X_SIZE, World_CHUNK_Y_SIZE, World_CHUNK_Z_SIZE, Array3DStoreOrder::YXZ>;
using World_Chunk_LightData  = Array3D<World_Light, World_CHUNK_X_SIZE, World_CHUNK_Y_SIZE, World_CHUNK_Z_SIZE, Array3DStoreOrder::YXZ>;
using World_Chunk_HeightData = Array2D<std::uint8_t, World_CHUNK_X_SIZE, World_CHUNK_Z_SIZE, Array2DStoreOrder::YX>;

struct World_Chunk_Storage
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
    // Stage==Empty: Initial stage of this chunk after allocation.
    // All allocated chunks are guaranteed to be associated with neighbours (Chunk holds valid neighbour chunks' pointer).
    Empty,

    // Stage==Generating: Workers are generating terrains/caves for this chunk.
    GenerationInProgress,
    GenerationComplete,

    // Stage==LocalLighting: Workers are flooding the chunk with initial lights.
    // For LocalLighting to start, all chunk neighbours must be in Stage==GenerationComplete.
    LocalLightingInProgress,
    LocalLightingComplete,

    // Stage==NeighbourLighting: This chunk is pending until all the neighbours become Stage==LocalLightingComplete.
    // This stage ensures that lights from neighbour chunks are also propagated into this chunk.
    NeighbourLightingInProgress,
    NeighbourLightingComplete,
};

struct World_Chunk
{
    const World_Chunk_ID ID;

    std::atomic<World_Chunk_Stage> Stage = World_Chunk_Stage::Empty;

    // Job deduplicate bitmask (GEN=1, LOCAL_LIGHT=2, NEIGHBOUR_LIGHT=4, MESH=8).
    // Stores the job type the chunk is currently queued for.
    // Example, when the chunk is in queue for JobType::Generation, EnqueuedStates |= GEN.
    // Example, when the chunk is poped out of queue for JobType::Generation, EnqueuedStates &= ~GEN.
    // This is to avoid duplicate enqueuing of jobs of same type.
    std::atomic<std::uint8_t> EnqueuedStates = 0;

    bool HasModified = false;

    std::array<World_Chunk*, (std::size_t)World_Chunk_Neighbour::COUNT> Neighbours{};
    std::atomic<bool> NeighboursSet = false;

    std::unique_ptr<World_Chunk_Storage> Storage;
    std::atomic<std::uint32_t>           StorageVersion = 0;

    explicit World_Chunk(World_Chunk_ID id) : ID{ id } {}

    World_Block GetBlockAt(World_LocalXYZ local) const;
    World_Light GetLightAt(World_LocalXYZ local) const;
    World_Light GetSunlightAt(World_LocalXYZ local) const;
    World_Light GetPointlightAt(World_LocalXYZ local) const;

    void SetBlockAt(World_LocalXYZ local, World_Block block);
    void SetLightAt(World_LocalXYZ local, World_Light sunlight, World_Light pointlight);
    void SetSunlightAt(World_LocalXYZ local, World_Light sunlight);
    void SetPointlightAt(World_LocalXYZ local, World_Light pointlight);

    int  GetHeightAt(int local_x, int local_z) const;
    int  GetMaxHeight() const;
    void SetHeightAt(int local_x, int local_z, std::uint8_t height);

    std::array<World_Block, static_cast<std::size_t>(World_Block_CrossNeighbour::Count)>
        GetCrossNeighbourBlocksAt(World_LocalXYZ local) const;
    std::array<World_Light, static_cast<std::size_t>(World_Block_CrossNeighbour::Count)>
        GetCrossNeighbourLightsAt(World_LocalXYZ local) const;
    std::array<World_Block, static_cast<std::size_t>(World_Block_WholeNeighbour::Count)>
        GetWholeNeighbourBlocksAt(World_LocalXYZ local) const;
};
