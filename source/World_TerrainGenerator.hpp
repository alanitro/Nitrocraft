#pragma once

struct Chunk;

void TerrainGenerator_Initialize(int world_seed);

void TerrainGenerator_GenerateTerrain(Chunk* chunk);
