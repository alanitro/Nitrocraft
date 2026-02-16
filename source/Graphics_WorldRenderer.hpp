#pragma once

#include <utility>
#include <vector>
#include "World_Coordinate.hpp"
#include "World_ChunkManager.hpp"
#include "Utility_Array2D.hpp"

class Camera;
struct World_Chunk;


void WorldRenderer_Initialize();
void WorldRenderer_Terminate();

void WorldRenderer_Render(const Camera& camera, float sunlight_intensity, glm::vec3 sky_color);
void WorldRenderer_PrepareChunksToRender(const std::vector<World_Chunk*>& chunks_in_render_area);
