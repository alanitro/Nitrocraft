#pragma once

#include <random>

struct Chunk;

class TerrainGenerator
{
public:
    TerrainGenerator() = default;
    ~TerrainGenerator() = default;
    TerrainGenerator(const TerrainGenerator&) = delete;
    TerrainGenerator& operator=(const TerrainGenerator&) = delete;

    void Initialize(std::uint32_t world_seed);

    void GenerateTerrain(Chunk* chunk);

private:
    std::mt19937 m_Rng;
};
