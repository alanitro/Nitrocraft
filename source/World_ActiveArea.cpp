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

                new_chunk->Payload = std::make_unique<World_Chunk_Payload>();

                chunk = new_chunk.get();

                chunk_map.emplace(chunk_id, std::move(new_chunk));
            }

            active_area.At(ix, iz) = chunk;
        }
    }

    for (int iz = 1; iz < World_LOADING_DIAMETER - 1; iz++)
    {
        for (int ix = 1; ix < World_LOADING_DIAMETER - 1; ix++)
        {
            World_Chunk* chunk = active_area.At(ix, iz);

            if (chunk->NeighboursSet) continue;

            chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XNZ0] = active_area.At(ix - 1, iz);
            chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XPZ0] = active_area.At(ix + 1, iz);
            chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZN] = active_area.At(ix, iz - 1);
            chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZP] = active_area.At(ix, iz + 1);
            chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XNZN] = active_area.At(ix - 1, iz - 1);
            chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XPZN] = active_area.At(ix + 1, iz - 1);
            chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XNZP] = active_area.At(ix - 1, iz + 1);
            chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XPZP] = active_area.At(ix + 1, iz + 1);

            chunk->NeighboursSet = true;
        }
    }
}

void World_ActiveArea_PerformGenerationPhase(World_ActiveArea& active_area)
{
    for (int iz = 1; iz < World_LOADING_DIAMETER - 1; iz++)
    {
        for (int ix = 1; ix < World_LOADING_DIAMETER - 1; ix++)
        {
            World_Chunk* chunk = active_area.At(ix, iz);

            if (chunk->GenerationComplete) continue;

            World_TerrainGeneration_GenerateChunk(chunk);

            chunk->GenerationComplete = true;
        }
    }
}

void World_ActiveArea_PerformLightingPhase(
    World_ActiveArea& active_area,
    std::queue<World_Lighting_LightAdditionNode>& sunlight_add_queue,
    std::queue<World_Lighting_LightRemovalNode>& sunlight_rem_queue,
    std::queue<World_Lighting_LightAdditionNode>& pointlight_add_queue,
    std::queue<World_Lighting_LightRemovalNode>& pointlight_rem_queue
)
{
    World_Lighting_UnpropagateSunlight(sunlight_rem_queue, sunlight_add_queue);

    World_Lighting_PropagateSunlight(sunlight_add_queue);

    World_Lighting_UnpropagatePointlight(pointlight_rem_queue, pointlight_add_queue);

    World_Lighting_PropagatePointlight(pointlight_add_queue);

    /*for (int az = 2; az < World_LOADING_DIAMETER - 2; az++)
    {
        for (int ax = 2; ax < World_LOADING_DIAMETER - 2; ax++)
        {
            World_Chunk* chunk = active_area.At(ax, az);

            if (chunk->LightingComplete) continue;

            for (int iz = 0; iz < World_CHUNK_Z_SIZE; iz++)
            {
                for (int ix = 0; ix < World_CHUNK_X_SIZE; ix++)
                {
                    for (int iy = World_CHUNK_Y_SIZE - 1; World_Block_IsOpaque(World_Chunk_GetBlockAt(chunk, World_LocalXYZ(ix, iy, iz))) == false; iy--)
                    {
                        World_Chunk_SetSunlightAt(chunk, World_LocalXYZ(ix, iy, iz), World_LIGHT_LEVEL_SUN);

                        sunlight_add_queue.emplace(chunk, World_LocalXYZ(ix, iy, iz));
                    }
                }
            }

            World_Lighting_PropagateSunlight(sunlight_add_queue);

            chunk->LightingComplete = true;
        }
    }*/

    for (int az = 2; az < World_LOADING_DIAMETER - 2; az++)
    {
        for (int ax = 2; ax < World_LOADING_DIAMETER - 2; ax++)
        {
            World_Chunk* chunk = active_area.At(ax, az);

            if (chunk->LightingComplete) continue;

            for (int iz = 0; iz < World_CHUNK_Z_SIZE; iz++)
            {
                for (int ix = 0; ix < World_CHUNK_X_SIZE; ix++)
                {
                    {// TODO: remove later (pointlight testing)
                        if (ix == World_CHUNK_X_SIZE / 2 && iz == World_CHUNK_Z_SIZE / 2) 
                        {
                            chunk->SetBlockAt(World_LocalXYZ(ix, 16, iz), World_Block(World_BlockID::GLOWSTONE));
                            chunk->SetBlockAt(World_LocalXYZ(ix, 32, iz), World_Block(World_BlockID::GLOWSTONE));
                            chunk->SetBlockAt(World_LocalXYZ(ix, 48, iz), World_Block(World_BlockID::GLOWSTONE));
                            chunk->SetBlockAt(World_LocalXYZ(ix, 64, iz), World_Block(World_BlockID::GLOWSTONE));
                            chunk->SetBlockAt(World_LocalXYZ(ix, 80, iz), World_Block(World_BlockID::GLOWSTONE));
                            chunk->SetPointlightAt(World_LocalXYZ(ix, 16, iz), World_LIGHT_LEVEL_14);
                            chunk->SetPointlightAt(World_LocalXYZ(ix, 32, iz), World_LIGHT_LEVEL_14);
                            chunk->SetPointlightAt(World_LocalXYZ(ix, 48, iz), World_LIGHT_LEVEL_14);
                            chunk->SetPointlightAt(World_LocalXYZ(ix, 64, iz), World_LIGHT_LEVEL_14);
                            chunk->SetPointlightAt(World_LocalXYZ(ix, 80, iz), World_LIGHT_LEVEL_14);
                            pointlight_add_queue.emplace(chunk, World_LocalXYZ(ix, 16, iz));
                            pointlight_add_queue.emplace(chunk, World_LocalXYZ(ix, 32, iz));
                            pointlight_add_queue.emplace(chunk, World_LocalXYZ(ix, 48, iz));
                            pointlight_add_queue.emplace(chunk, World_LocalXYZ(ix, 64, iz));
                            pointlight_add_queue.emplace(chunk, World_LocalXYZ(ix, 80, iz));
                        }
                    }

                    for (int iy = World_CHUNK_Y_SIZE - 1; chunk->GetBlockAt(World_LocalXYZ(ix, iy, iz)).IsOpaque() == false; iy--)
                    {
                        chunk->SetSunlightAt(World_LocalXYZ(ix, iy, iz), World_LIGHT_LEVEL_SUN);

                        sunlight_add_queue.emplace(chunk, World_LocalXYZ(ix, iy, iz));
                    }
                }
            }

            World_Lighting_PropagateSunlight(sunlight_add_queue);

            World_Lighting_PropagatePointlight(pointlight_add_queue); // TODO: remove later (pointlight testing)

            chunk->LightingComplete = true;
        }
    }
}


