#pragma once

#include <FastNoise/FastNoise.h>
#include "Utility_Array2D.hpp"
#include "Utility_Array3D.hpp"
#include "World_Coordinate.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"

class World_Chunk;

namespace nitrocraft::world
{

class TerrainGenerator
{
public:
    void Initialize(int generation_seed = 12345);

    void GenerateTerrain(Chunk* chunk);

private:
    struct SampleBuffers
    {
        utility::Array2D<float, CHUNK_X_SIZE, CHUNK_Z_SIZE> continentalness;
        utility::Array3D<float, CHUNK_X_SIZE, CHUNK_Y_SIZE, CHUNK_Z_SIZE> cheese_cavern;
        utility::Array3D<float, CHUNK_X_SIZE, CHUNK_Y_SIZE, CHUNK_Z_SIZE> spaghetti_cavern1;
        utility::Array3D<float, CHUNK_X_SIZE, CHUNK_Y_SIZE, CHUNK_Z_SIZE> spaghetti_cavern2;
    };

    int m_generation_seed;

    FastNoise::SmartNode<FastNoise::FractalFBm> m_continentalness_noise;
    FastNoise::SmartNode<FastNoise::FractalFBm> m_cheese_noise;
    FastNoise::SmartNode<FastNoise::FractalFBm> m_spaghetti_noise1;
    FastNoise::SmartNode<FastNoise::FractalFBm> m_spaghetti_noise2;

    void PopulateSamples(GlobalXYZ chunk_offset, SampleBuffers& samples);
    void PopulateBlocks(Chunk* chunk, SampleBuffers& samples);
    void PopulateHeights(Chunk* chunk);
};

} // namespace nitrocraft::world
