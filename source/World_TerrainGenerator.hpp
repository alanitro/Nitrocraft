#pragma once

#include "World_Definitions.hpp"

struct Chunk;

namespace TerrainGenerator
{
    void Initialize(int world_seed);

    void GenerateTerrain(Chunk* chunk);
}
