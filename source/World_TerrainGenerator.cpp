#include "World_TerrainGenerator.hpp"

#include <algorithm>
#include <FastNoise/FastNoise.h>
#include "Utility_Array2D.hpp"
#include "Utility_Array3D.hpp"
#include "World_Definitions.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"

namespace
{
    constexpr float CONTINENTALNESS_FREQENCY = 0.0016f;
    constexpr float CAVERN_FREQUENCY = 0.008f;

    constexpr std::size_t NOISE_MAP_X_SIZE = WORLD_CHUNK_X_SIZE;
    constexpr std::size_t NOISE_MAP_Y_SIZE = WORLD_CHUNK_Y_SIZE;
    constexpr std::size_t NOISE_MAP_Z_SIZE = WORLD_CHUNK_Z_SIZE;

    int WorldSeed;

    FastNoise::SmartNode<FastNoise::FractalFBm> ContinentalnessNoise{};
    FastNoise::SmartNode<FastNoise::FractalFBm> CaveNoise{};
}

void TerrainGenerator::Initialize(int world_seed)
{
    WorldSeed = world_seed;

    auto simplex_node = FastNoise::New<FastNoise::SuperSimplex>();
    simplex_node->SetScale(50.0f);

    ContinentalnessNoise = FastNoise::New<FastNoise::FractalFBm>();
    //ContinentalnessNoise->SetSource(FastNoise::New<FastNoise::SuperSimplex>());
    ContinentalnessNoise->SetSource(simplex_node);
    /*ContinentalnessNoise->SetOctaveCount(3);
    ContinentalnessNoise->SetLacunarity(3.6f);
    ContinentalnessNoise->SetGain(0.4f);*/

    CaveNoise = FastNoise::New<FastNoise::FractalFBm>();
    //CaveNoise->SetSource(FastNoise::New<FastNoise::SuperSimplex>());
    CaveNoise->SetSource(simplex_node);
    /*CaveNoise->SetOctaveCount(3);
    CaveNoise->SetLacunarity(3.0f);
    CaveNoise->SetGain(0.2f);*/
}

//void TerrainGenerator::GenerateTerrain(WorldXYZ chunk_offset, Chunk* chunk)
//{
//    const int& cx = chunk_offset.x;
//    const int& cy = chunk_offset.y;
//    const int& cz = chunk_offset.z;
//
//    // Populate noise maps
//    static Array2D<float, NOISE_MAP_X_SIZE, NOISE_MAP_Z_SIZE> continentalness_samples{};
//    ContinentalnessNoise->GenUniformGrid2D(
//        continentalness_samples.Data(),
//        chunk_offset.x, chunk_offset.z,
//        NOISE_MAP_X_SIZE, NOISE_MAP_Z_SIZE,
//        1.0f, 1.0f,
//        WorldSeed
//    );
//
//    static Array3D<float, NOISE_MAP_X_SIZE, NOISE_MAP_Y_SIZE, NOISE_MAP_Z_SIZE, Array3DStoreOrder::YXZ> cavern_samples{};
//    static Array3D<float, NOISE_MAP_X_SIZE, NOISE_MAP_Y_SIZE, NOISE_MAP_Z_SIZE> temp{};
//
//    CaveNoise->GenUniformGrid3D(
//        temp.Data(),
//        chunk_offset.x, chunk_offset.y, chunk_offset.z,
//        NOISE_MAP_X_SIZE, NOISE_MAP_Y_SIZE, NOISE_MAP_Z_SIZE,
//        1.0f, 1.0f, 1.0f,
//        WorldSeed
//    );
//
//    for (int iz = 0; iz < NOISE_MAP_Z_SIZE; iz++)
//    {
//        for (int ix = 0; ix < NOISE_MAP_X_SIZE; ix++)
//        {
//            for (int iy = 0; iy < NOISE_MAP_Y_SIZE; iy++)
//            {
//                cavern_samples.At(ix, iy, iz) = temp.At(ix, iy, iz);
//            }
//        }
//    }
//
//    // Generate terrain
//    for (int iz = 0; iz < WORLD_CHUNK_Z_SIZE; iz++)
//    {
//        for (int ix = 0; ix < WORLD_CHUNK_X_SIZE; ix++)
//        {
//            const int height = static_cast<int>(std::floor(continentalness_samples.At(ix, iz) * 64 + WORLD_SEA_LEVEL + 16));
//
//            chunk->Blocks.At(ix, 0, iz) = BlockID::BEDROCK;
//
//            for (int iy = 1; iy < WORLD_CHUNK_Y_SIZE; iy++)
//            {
//                Block& block = chunk->Blocks.At(ix, iy, iz);
//
//                float sample = cavern_samples.At(ix, iy, iz);
//
//                float skew = (static_cast<float>(iy) / static_cast<float>(WORLD_HEIGHT / 2)) * 0.12f;
//
//                bool solid = (0.88f + skew) > sample;
//
//                if (iy > height || !solid)
//                {
//                    block = BlockID::AIR;
//
//                    continue;
//                }
//
//                if (iy < height - 3)
//                {
//                    block = BlockID::STONE;
//
//                    continue;
//                }
//
//                block = BlockID::GRASS;
//            }
//        }
//    }
//}

void TerrainGenerator::GenerateTerrain(WorldXYZ chunk_offset, Chunk* chunk)
{
    const int& cx = chunk_offset.x;
    const int& cy = chunk_offset.y;
    const int& cz = chunk_offset.z;

    // Populate noise maps
    static Array3D<float, NOISE_MAP_X_SIZE, NOISE_MAP_Y_SIZE, NOISE_MAP_Z_SIZE, Array3DStoreOrder::YXZ> cavern_samples{};
    static Array3D<float, NOISE_MAP_X_SIZE, NOISE_MAP_Y_SIZE, NOISE_MAP_Z_SIZE> temp{};

    CaveNoise->GenUniformGrid3D(
        temp.Data(),
        chunk_offset.x, chunk_offset.y, chunk_offset.z,
        NOISE_MAP_X_SIZE, NOISE_MAP_Y_SIZE, NOISE_MAP_Z_SIZE,
        1.0f, 1.0f, 1.0f,
        WorldSeed
    );

    for (int iz = 0; iz < NOISE_MAP_Z_SIZE; iz++)
    {
        for (int ix = 0; ix < NOISE_MAP_X_SIZE; ix++)
        {
            for (int iy = 0; iy < NOISE_MAP_Y_SIZE; iy++)
            {
                cavern_samples.At(ix, iy, iz) = temp.At(ix, iy, iz);
            }
        }
    }

    // Generate terrain
    for (int iz = 0; iz < WORLD_CHUNK_Z_SIZE; iz++)
    {
        for (int ix = 0; ix < WORLD_CHUNK_X_SIZE; ix++)
        {
            chunk->Blocks.At(ix, 0, iz) = BlockID::BEDROCK;

            for (int iy = 1; iy < WORLD_CHUNK_Y_SIZE; iy++)
            {
                Block& block = chunk->Blocks.At(ix, iy, iz);

                block = cavern_samples.At(ix,iy,iz) > 0.0f ? BlockID::STONE : BlockID::AIR;
            }
        }
    }
}