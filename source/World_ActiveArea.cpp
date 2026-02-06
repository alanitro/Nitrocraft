#include "World_ActiveArea.hpp"

#include <memory>
#include <queue>
#include <algorithm>

void World_ActiveArea_LoadChunks(
    World_ActiveArea& active_area,
    std::unordered_map<World_ChunkID, std::unique_ptr<World_Chunk>>& chunk_map,
    World_ChunkID center_chunk_id
)
{
    const World_ChunkID chunk_offset_id = center_chunk_id - World_ChunkID(World_LOADING_RADIUS, 0, World_LOADING_RADIUS);

    for (int iz = 0; iz < World_LOADING_DIAMETER; iz++)
    {
        for (int ix = 0; ix < World_LOADING_DIAMETER; ix++)
        {
            auto chunk_id = World_ChunkID(chunk_offset_id + World_ChunkID(ix, 0, iz));

            World_Chunk* chunk = nullptr;

            if (auto iter = chunk_map.find(chunk_id); iter != chunk_map.end())
            {
                chunk = iter->second.get();
            }
            else
            {
                auto new_chunk = std::make_unique<World_Chunk>(chunk_id);

                new_chunk->Payload = std::make_unique_for_overwrite<World_Chunk_Payload>();

                chunk = new_chunk.get();

                chunk_map.emplace(chunk_id, std::move(new_chunk));
            }

            active_area.At(ix, iz) = chunk;
        }
    }

    // Generation phase
    for (int iz = 1; iz < World_LOADING_DIAMETER - 1; iz++)
    {
        for (int ix = 1; ix < World_LOADING_DIAMETER - 1; ix++)
        {
            World_Chunk* chunk = active_area.At(ix, iz);

            if (chunk->NeighboursSet == false)
            {
                chunk->NeighbourXNZ0 = active_area.At(ix - 1, iz);
                chunk->NeighbourXPZ0 = active_area.At(ix + 1, iz);
                chunk->NeighbourX0ZN = active_area.At(ix, iz - 1);
                chunk->NeighbourX0ZP = active_area.At(ix, iz + 1);
                chunk->NeighbourXNZN = active_area.At(ix - 1, iz - 1);
                chunk->NeighbourXPZN = active_area.At(ix + 1, iz - 1);
                chunk->NeighbourXNZP = active_area.At(ix - 1, iz + 1);
                chunk->NeighbourXPZP = active_area.At(ix + 1, iz + 1);

                chunk->NeighboursSet = true;
            }

            if (chunk->GenerationComplete) continue;

            World_TerrainGeneration_GenerateChunk(chunk);

            chunk->GenerationComplete = true;
        }
    }
}
