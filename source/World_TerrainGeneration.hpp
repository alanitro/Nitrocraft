#pragma once

struct World_Chunk;

void World_TerrainGeneration_Initialize(int generation_seed);

void World_TerrainGeneration_GenerateChunk(World_Chunk* chunk);
