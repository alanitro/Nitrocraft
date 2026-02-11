#pragma once

#include <queue>
#include <unordered_map>
#include "World_Definitions.hpp"
#include "World_Chunk.hpp"
#include "World_Generation.hpp"
#include "World_Lighting.hpp"
#include "Utility_Array2D.hpp"

// World Active Area -> [0, World_LOADING_DIAMETER]^2 area of loaded chunks

// World Generation Area -> [1, World_LOADING_DIAMETER - 1]^2 subset of Active Area
// Terrain generation takes place for chunks in Generation Area.

// World Lighting Area -> [2, World_LOADING_DIAMETER - 2]^2 subset of Active Area
// Once the terrain generation is done, chunk lighting is done for chunks in Lighting Area.

// World Meshing Area -> [3, World_LOADING_DIAMETER - 3]^2 subset of Active Area
// After lighting is done, chunk meshing is done for chunks in Meshing Area.

struct World_Chunk;

using World_ActiveArea = Array2D<World_Chunk*, World_LOADING_DIAMETER, World_LOADING_DIAMETER>;

void World_ActiveArea_LoadChunks(
    World_ActiveArea& active_area,
    std::unordered_map<World_ChunkID, std::unique_ptr<World_Chunk>>& chunk_map,
    World_ChunkID center_chunk_id
);

void World_ActiveArea_PerformGenerationPhase(
    World_ActiveArea& active_area
);

void World_ActiveArea_PerformLightingPhase(
    World_ActiveArea& active_area,
    std::queue<World_Lighting_LightAdditionNode>& sunlight_add_queue,
    std::queue<World_Lighting_LightRemovalNode>& sunlight_rem_queue,
    std::queue<World_Lighting_LightAdditionNode>& pointlight_add_queue,
    std::queue<World_Lighting_LightRemovalNode>& pointlight_rem_queue
);
