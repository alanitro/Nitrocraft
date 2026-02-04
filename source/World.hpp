#pragma once

#include "World_Definitions.hpp"
#include "World_Block.hpp"
#include "Utility_Array2D.hpp"

class Camera;
struct World_Chunk;

void World_Initialize();
void World_Terminate();
void World_Update(const Camera& camera);

World_Block World_GetBlockAt(World_GlobalXYZ position);

const World_Chunk* World_GetChunkAt(World_GlobalXYZ position);

const Array2D<World_Chunk*, World_LOADING_DIAMETER, World_LOADING_DIAMETER>& World_GetActiveArea();
