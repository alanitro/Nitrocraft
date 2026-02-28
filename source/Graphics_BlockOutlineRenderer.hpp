#pragma once

#include "World_Coordinate.hpp"
#include "Graphics_Camera.hpp"

void Graphics_BlockOutlineRenderer_Initialize();
void Graphics_BlockOutlineRenderer_Terminate();

void Graphics_BlockOutlineRenderer_Render(const Camera& camera, World_Position block_position);
