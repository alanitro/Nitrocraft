#pragma once

#include <memory>
#include <unordered_map>
#include "World_Definitions.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"
#include "World_TerrainGenerator.hpp"
#include "Utility_Array2D.hpp"

class Camera;

namespace World
{
    void Initialize();
    void Terminate();
    void Update(Camera& camera);

    Block GetBlockAt(WorldPosition position);

    Chunk* GetChunkAt(WorldPosition position);

    const Array2D<Chunk*, WORLD_LOADING_DIAMETER, WORLD_LOADING_DIAMETER>& GetActiveArea();
};
