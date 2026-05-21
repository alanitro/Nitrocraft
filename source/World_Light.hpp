#pragma once

#include <queue>
#include "World_Coordinate.hpp"

namespace nitrocraft::world
{

struct Chunk;

// Light Constants
using LightLevel = std::uint8_t; // ppppssss p = Pointlight, s = Sunlight 

constexpr LightLevel LIGHT_LEVEL_00 = 0x00;
constexpr LightLevel LIGHT_LEVEL_01 = 0x01;
constexpr LightLevel LIGHT_LEVEL_02 = 0x02;
constexpr LightLevel LIGHT_LEVEL_03 = 0x03;
constexpr LightLevel LIGHT_LEVEL_04 = 0x04;
constexpr LightLevel LIGHT_LEVEL_05 = 0x05;
constexpr LightLevel LIGHT_LEVEL_06 = 0x06;
constexpr LightLevel LIGHT_LEVEL_07 = 0x07;
constexpr LightLevel LIGHT_LEVEL_08 = 0x08;
constexpr LightLevel LIGHT_LEVEL_09 = 0x09;
constexpr LightLevel LIGHT_LEVEL_10 = 0x0A;
constexpr LightLevel LIGHT_LEVEL_11 = 0x0B;
constexpr LightLevel LIGHT_LEVEL_12 = 0x0C;
constexpr LightLevel LIGHT_LEVEL_13 = 0x0D;
constexpr LightLevel LIGHT_LEVEL_14 = 0x0E;
constexpr LightLevel LIGHT_LEVEL_15 = 0x0F;
constexpr LightLevel LIGHT_LEVEL_MIN = LIGHT_LEVEL_00;
constexpr LightLevel LIGHT_LEVEL_MAX = LIGHT_LEVEL_15;
constexpr LightLevel LIGHT_LEVEL_SUN = LIGHT_LEVEL_MAX;
constexpr LightLevel LIGHT_LEVEL_POINT = LIGHT_LEVEL_MAX;

constexpr LightLevel ExtractSunlight(LightLevel light)
{
    return (light >> 0) & 0x0F;
}

constexpr LightLevel ExtractPointlight(LightLevel light)
{
    return (light >> 4) & 0x0F;
}

// Light Propagation Definitions
struct LightAdditionNode
{
    Chunk*   chunk;
    LocalXYZ local;
};

struct LightRemovalNode
{
    Chunk*     chunk;
    LocalXYZ   local;
    LightLevel light;
};

void PropagateSunlight(
    std::queue<LightAdditionNode>& sunlight_add_queue
);

void UnpropagateSunlight(
    std::queue<LightRemovalNode>& sunlight_rem_queue,
    std::queue<LightAdditionNode>& sunlight_add_queue
);

void PropagatePointlight(
    std::queue<LightAdditionNode>& pointlight_add_queue
);

void UnpropagatePointlight(
    std::queue<LightRemovalNode>& pointlight_rem_queue,
    std::queue<LightAdditionNode>& pointlight_add_queue
);

void PropagateInitialSunlight(Chunk* chunk);

} // namespace nitrocraft::world
