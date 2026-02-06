#pragma once

#include "World_Definitions.hpp"
#include "World_ActiveArea.hpp"
#include "World_Block.hpp"
#include "Utility_Array2D.hpp"

class Camera;
struct World_Chunk;

void        World_Initialize();
void        World_Terminate();
void        World_Update(const Camera& camera);

float       World_GetSunlightIntensity();
glm::vec3   World_GetSkyColor();

World_Block World_GetBlockAt(World_GlobalXYZ position);

const World_Chunk* World_GetChunkAt(World_GlobalXYZ position);

const World_ActiveArea& World_GetActiveArea();
