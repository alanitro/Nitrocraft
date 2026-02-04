#include "World.hpp"

#include <memory>
#include <unordered_map>
#include "Graphics_Camera.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"
#include "World_TerrainGeneration.hpp"

namespace
{
    std::unordered_map<World_ChunkID, std::unique_ptr<World_Chunk>>         World_ChunkMap;
    Array2D<World_Chunk*, World_LOADING_DIAMETER, World_LOADING_DIAMETER>   World_ActiveArea{};

    void Update_ActiveArea(World_ChunkID center_chunk_id);
}

void World_Initialize()
{
    World_TerrainGeneration_Initialize(12345);

    Update_ActiveArea(World_ChunkID{ 0,0,0 });
}

void World_Terminate()
{
    World_ChunkMap.clear();
}

void World_Update(const Camera& camera)
{
    Update_ActiveArea(World_FromGlobalToChunkID(camera.GetPosition()));
}

World_Block World_GetBlockAt(World_GlobalXYZ position)
{
    auto chunk = World_GetChunkAt(position);

    if (chunk == nullptr) return World_Block{ World_Block_ID::AIR };

    return World_Chunk_GetBlockAt(chunk, World_FromGlobalToLocal(position));
}

const World_Chunk* World_GetChunkAt(World_GlobalXYZ position)
{
    auto iter = World_ChunkMap.find(World_FromGlobalToChunkID(position));

    if (iter == World_ChunkMap.end()) return nullptr;

    return iter->second.get();
}

const Array2D<World_Chunk*, World_LOADING_DIAMETER, World_LOADING_DIAMETER>& World_GetActiveArea()
{
    return World_ActiveArea;
}

namespace
{
void Update_ActiveArea(World_ChunkID center_chunk_id)
{
    const World_ChunkID chunk_offset_id = center_chunk_id - World_ChunkID(World_LOADING_RADIUS, 0, World_LOADING_RADIUS);

    for (int iz = 0; iz < World_LOADING_DIAMETER; iz++)
    {
        for (int ix = 0; ix < World_LOADING_DIAMETER; ix++)
        {
            auto chunk_id = World_ChunkID(chunk_offset_id + World_ChunkID(ix, 0, iz));

            auto iter = World_ChunkMap.find(chunk_id);

            if (iter != World_ChunkMap.end())
            {
                World_ActiveArea.At(ix, iz) = iter->second.get();

                continue;
            }
            else
            {
                World_ChunkMap.erase(chunk_id);
            }

            auto [it, inserted] = World_ChunkMap.emplace(chunk_id, std::make_unique<World_Chunk>(chunk_id));

            World_Chunk* new_chunk = it->second.get();

            World_TerrainGeneration_GenerateChunk(new_chunk);

            World_ActiveArea.At(ix, iz) = new_chunk;
        }
    }

    for (int iz = 1; iz < World_LOADING_DIAMETER - 1; iz++)
    {
        for (int ix = 1; ix < World_LOADING_DIAMETER - 1; ix++)
        {
            World_Chunk* chunk = World_ActiveArea.At(ix, iz);
            chunk->NeighbourXNZ0 = World_ActiveArea.At(ix - 1, iz);
            chunk->NeighbourXPZ0 = World_ActiveArea.At(ix + 1, iz);
            chunk->NeighbourX0ZN = World_ActiveArea.At(ix, iz - 1);
            chunk->NeighbourX0ZP = World_ActiveArea.At(ix, iz + 1);
            chunk->NeighbourXNZN = World_ActiveArea.At(ix - 1, iz - 1);
            chunk->NeighbourXPZN = World_ActiveArea.At(ix + 1, iz - 1);
            chunk->NeighbourXNZP = World_ActiveArea.At(ix - 1, iz + 1);
            chunk->NeighbourXPZP = World_ActiveArea.At(ix + 1, iz + 1);
            chunk->NeighboursSet = true;
        }
    }
}
}