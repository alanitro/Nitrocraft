#pragma once

#include <cstdint>
#include <vector>

struct World_Chunk; 
struct World_Chunk_CPUMesh;

void World_Mesh_GenerateChunkCPUMesh(const World_Chunk* chunk, World_Chunk_CPUMesh& mesh);
