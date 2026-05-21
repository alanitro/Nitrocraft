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

namespace nitrocraft::world
{

using ChunkBlockData  = utility::Array3D<Block, CHUNK_X_SIZE, CHUNK_Y_SIZE, CHUNK_Z_SIZE, utility::Array3DStoreOrder::YXZ>;
using ChunkLightData  = utility::Array3D<LightLevel, CHUNK_X_SIZE, CHUNK_Y_SIZE, CHUNK_Z_SIZE, utility::Array3DStoreOrder::YXZ>;
using ChunkHeightData = utility::Array2D<std::uint8_t, CHUNK_X_SIZE, CHUNK_Z_SIZE, utility::Array2DStoreOrder::YX>;

struct ChunkStorage
{
    ChunkBlockData  blocks;
    ChunkLightData  lights;
    ChunkHeightData heights;
};

enum class ChunkNeighbour
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

enum class ChunkStage
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

struct Chunk
{
    const ChunkID id;

    std::atomic<ChunkStage> stage = ChunkStage::Empty;

    // Job deduplicate bitmask (GEN=1, LOCAL_LIGHT=2, NEIGHBOUR_LIGHT=4, MESH=8).
    // Stores the job type the chunk is currently queued for.
    // Example, when the chunk is in queue for JobType::Generation, enqueued_states |= GEN.
    // Example, when the chunk is poped out of queue for JobType::Generation, enqueued_states &= ~GEN.
    // This is to avoid duplicate enqueuing of jobs of same type.
    std::atomic<std::uint8_t> enqueued_states = 0;

    bool has_modified = false;

    std::array<Chunk*, (std::size_t)ChunkNeighbour::COUNT> neighbours{};
    std::atomic<bool> neighbours_set = false;

    std::unique_ptr<ChunkStorage>   storage;
    std::atomic<std::uint32_t>      storage_version = 0;

    explicit Chunk(ChunkID id) : id{ id } {}

    world::Block GetBlockAt(LocalXYZ local) const;
    LightLevel GetLightAt(LocalXYZ local) const;
    LightLevel GetSunlightAt(LocalXYZ local) const;
    LightLevel GetPointlightAt(LocalXYZ local) const;

    void SetBlockAt(LocalXYZ local, Block block);
    void SetLightAt(LocalXYZ local, LightLevel sunlight, LightLevel pointlight);
    void SetSunlightAt(LocalXYZ local, LightLevel sunlight);
    void SetPointlightAt(LocalXYZ local, LightLevel pointlight);

    int  GetHeightAt(int local_x, int local_z) const;
    int  GetMaxHeight() const;

    std::array<Block, static_cast<std::size_t>(BlockCrossNeighbour::Count)>
        GetCrossNeighbourBlocksAt(LocalXYZ local) const;
    std::array<LightLevel, static_cast<std::size_t>(BlockCrossNeighbour::Count)>
        GetCrossNeighbourLightsAt(LocalXYZ local) const;
    std::array<Block, static_cast<std::size_t>(BlockWholeNeighbour::Count)>
        GetWholeNeighbourBlocksAt(LocalXYZ local) const;
};

} // namespace nitrocraft::world
