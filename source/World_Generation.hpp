#pragma once

struct World_Chunk;

void World_Generation_Initialize(int generation_seed);

void World_Generation_GenerateChunk(World_Chunk* chunk);
