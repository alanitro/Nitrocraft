#pragma once

#include <queue>
#include "World_Coordinate.hpp"

struct World_Chunk;

// Light Constants
using World_Light = std::uint8_t; // ppppssss p = Pointlight, s = Sunlight 

constexpr World_Light World_LIGHT_LEVEL_00 = 0x00;
constexpr World_Light World_LIGHT_LEVEL_01 = 0x01;
constexpr World_Light World_LIGHT_LEVEL_02 = 0x02;
constexpr World_Light World_LIGHT_LEVEL_03 = 0x03;
constexpr World_Light World_LIGHT_LEVEL_04 = 0x04;
constexpr World_Light World_LIGHT_LEVEL_05 = 0x05;
constexpr World_Light World_LIGHT_LEVEL_06 = 0x06;
constexpr World_Light World_LIGHT_LEVEL_07 = 0x07;
constexpr World_Light World_LIGHT_LEVEL_08 = 0x08;
constexpr World_Light World_LIGHT_LEVEL_09 = 0x09;
constexpr World_Light World_LIGHT_LEVEL_10 = 0x0A;
constexpr World_Light World_LIGHT_LEVEL_11 = 0x0B;
constexpr World_Light World_LIGHT_LEVEL_12 = 0x0C;
constexpr World_Light World_LIGHT_LEVEL_13 = 0x0D;
constexpr World_Light World_LIGHT_LEVEL_14 = 0x0E;
constexpr World_Light World_LIGHT_LEVEL_15 = 0x0F;
constexpr World_Light World_LIGHT_LEVEL_MIN = World_LIGHT_LEVEL_00;
constexpr World_Light World_LIGHT_LEVEL_MAX = World_LIGHT_LEVEL_15;
constexpr World_Light World_LIGHT_LEVEL_SUN = World_LIGHT_LEVEL_MAX;
constexpr World_Light World_LIGHT_LEVEL_POINT = World_LIGHT_LEVEL_MAX;

constexpr World_Light World_ExtractSunlight(World_Light light)
{
    return (light >> 0) & 0x0F;
}

constexpr World_Light World_ExtractPointlight(World_Light light)
{
    return (light >> 4) & 0x0F;
}

// Light Propagation Definitions
struct World_Light_LightAdditionNode
{
    World_Chunk*    Chunk;
    World_LocalXYZ  Local;
};

struct World_Light_LightRemovalNode
{
    World_Chunk*    Chunk;
    World_LocalXYZ  Local;
    World_Light     Light;
};

void World_Light_PropagateSunlight(
    std::queue<World_Light_LightAdditionNode>& sunlight_add_queue
);

void World_Light_UnpropagateSunlight(
    std::queue<World_Light_LightRemovalNode>& sunlight_rem_queue,
    std::queue<World_Light_LightAdditionNode>& sunlight_add_queue
);

void World_Light_PropagatePointlight(
    std::queue<World_Light_LightAdditionNode>& pointlight_add_queue
);

void World_Light_UnpropagatePointlight(
    std::queue<World_Light_LightRemovalNode>& pointlight_rem_queue,
    std::queue<World_Light_LightAdditionNode>& pointlight_add_queue
);
