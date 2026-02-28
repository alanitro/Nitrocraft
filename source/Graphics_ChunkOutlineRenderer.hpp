#pragma once

#include "World_Coordinate.hpp"
#include "Graphics_Camera.hpp"

void Graphics_ChunkOutlineRenderer_Initialize();
void Graphics_ChunkOutlineRenderer_Terminate();

void Graphics_ChunkOutlineRenderer_Render(const Camera& camera, World_Position chunk_offset);
