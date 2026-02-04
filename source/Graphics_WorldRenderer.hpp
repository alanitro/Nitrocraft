#pragma once

#include "World_Definitions.hpp"
#include "Utility_Array2D.hpp"

class Camera;
struct Chunk;

void WorldRenderer_Initialize();
void WorldRenderer_Terminate();

void WorldRenderer_Render(const Camera& camera);
void WorldRenderer_PrepareChunksToRender(const Array2D<Chunk*, WORLD_LOADING_DIAMETER, WORLD_LOADING_DIAMETER>& active_area);
