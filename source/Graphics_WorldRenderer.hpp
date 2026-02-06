#pragma once

#include "World_Definitions.hpp"
#include "World_ActiveArea.hpp"
#include "Utility_Array2D.hpp"

class Camera;
struct World_Chunk;


void WorldRenderer_Initialize();
void WorldRenderer_Terminate();

void WorldRenderer_Render(const Camera& camera, float sunlight_intensity, glm::vec3 sky_color);
void WorldRenderer_PrepareChunksToRender(const World_ActiveArea& active_area);
