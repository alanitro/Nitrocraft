#pragma once

#include <queue>
#include "World_Definitions.hpp"

struct World_Chunk;

struct World_Lighting_LightAdditionNode
{
    World_Chunk*    Chunk;
    World_LocalXYZ  Local;
};

struct World_Lighting_LightRemovalNode
{
    World_Chunk*    Chunk;
    World_LocalXYZ  Local;
    World_Light     Light;
};

void World_Lighting_PropagateSunlight(
    std::queue<World_Lighting_LightAdditionNode>& sunlight_add_queue
);

void World_Lighting_UnpropagateSunlight(
    std::queue<World_Lighting_LightRemovalNode>& sunlight_rem_queue,
    std::queue<World_Lighting_LightAdditionNode>& sunlight_add_queue
);

void World_Lighting_PropagatePointlight(
    std::queue<World_Lighting_LightAdditionNode>& pointlight_add_queue
);

void World_Lighting_UnpropagatePointlight(
    std::queue<World_Lighting_LightRemovalNode>& pointlight_rem_queue,
    std::queue<World_Lighting_LightAdditionNode>& pointlight_add_queue
);
