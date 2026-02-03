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
    constexpr float CONTINENTALNESS_SCALE  = 625.0f;
    constexpr float CHEESE_CAVERN_SCALE    = 140.0f;
    constexpr float SPAGHETTI_CAVERN_SCALE = 160.0f;

    constexpr std::size_t NOISE_SAMPLE_X_SIZE = WORLD_CHUNK_X_SIZE;
    constexpr std::size_t NOISE_SAMPLE_Y_SIZE = WORLD_CHUNK_Y_SIZE;
    constexpr std::size_t NOISE_SAMPLE_Z_SIZE = WORLD_CHUNK_Z_SIZE;

    int WorldSeed;

    // Terrain noise
    FastNoise::SmartNode<FastNoise::FractalFBm>     ContinentalnessNoise{};

    // Cavern noise
    FastNoise::SmartNode<FastNoise::FractalFBm>  CheeseCavernNoise{};
    FastNoise::SmartNode<FastNoise::FractalFBm>  SpaghettiCavernNoise1{};
    FastNoise::SmartNode<FastNoise::FractalFBm>  SpaghettiCavernNoise2{};

    // Noise sample storage
    Array2D<float, NOISE_SAMPLE_X_SIZE, NOISE_SAMPLE_Z_SIZE>                        ContinentalnessSamples;
    Array3D<float, NOISE_SAMPLE_X_SIZE, NOISE_SAMPLE_Y_SIZE, NOISE_SAMPLE_Z_SIZE>   CheeseCavernSamples;
    Array3D<float, NOISE_SAMPLE_X_SIZE, NOISE_SAMPLE_Y_SIZE, NOISE_SAMPLE_Z_SIZE>   SpaghettiCavernSamples1;
    Array3D<float, NOISE_SAMPLE_X_SIZE, NOISE_SAMPLE_Y_SIZE, NOISE_SAMPLE_Z_SIZE>   SpaghettiCavernSamples2;

    void GenerateSamples(WorldPosition chunk_offset)
    {
        float cx = static_cast<float>(chunk_offset.x);
        float cy = static_cast<float>(chunk_offset.y);
        float cz = static_cast<float>(chunk_offset.z);

        ContinentalnessNoise->GenUniformGrid2D(
            ContinentalnessSamples.Data(),
            cx, cz,
            NOISE_SAMPLE_X_SIZE, NOISE_SAMPLE_Z_SIZE,
            1.0f, 1.0f,
            WorldSeed
        );

        CheeseCavernNoise->GenUniformGrid3D(
            CheeseCavernSamples.Data(),
            cx, cy, cz,
            NOISE_SAMPLE_X_SIZE, NOISE_SAMPLE_Y_SIZE, NOISE_SAMPLE_Z_SIZE,
            1.0f, 1.0f, 1.0f,
            WorldSeed
        );

        SpaghettiCavernNoise1->GenUniformGrid3D(
            SpaghettiCavernSamples1.Data(),
            cx, cy, cz,
            NOISE_SAMPLE_X_SIZE, NOISE_SAMPLE_Y_SIZE, NOISE_SAMPLE_Z_SIZE,
            1.0f, 1.0f, 1.0f,
            WorldSeed + 10000
        );

        SpaghettiCavernNoise2->GenUniformGrid3D(
            SpaghettiCavernSamples2.Data(),
            cx, cy, cz,
            NOISE_SAMPLE_X_SIZE, NOISE_SAMPLE_Y_SIZE, NOISE_SAMPLE_Z_SIZE,
            1.0f, 1.0f, 1.0f,
            WorldSeed + 20000
        );
    }
}

void TerrainGenerator::Initialize(int world_seed)
{
    WorldSeed = world_seed;

    {
        auto continentalness_source = FastNoise::New<FastNoise::SuperSimplex>();
        continentalness_source->SetScale(CONTINENTALNESS_SCALE);

        ContinentalnessNoise = FastNoise::New<FastNoise::FractalFBm>();
        ContinentalnessNoise->SetSource(continentalness_source);
        ContinentalnessNoise->SetOctaveCount(4);
        ContinentalnessNoise->SetLacunarity(2.6f);
        ContinentalnessNoise->SetGain(0.5f);
    }

    {
        auto cheese_cavern_source = FastNoise::New<FastNoise::Simplex>();
        cheese_cavern_source->SetScale(CHEESE_CAVERN_SCALE);

        auto domain_axis_scale = FastNoise::New<FastNoise::DomainAxisScale>();
        domain_axis_scale->SetSource(cheese_cavern_source);
        domain_axis_scale->SetScaling<FastNoise::Dim::X>(1.f);
        domain_axis_scale->SetScaling<FastNoise::Dim::Z>(1.f);
        domain_axis_scale->SetScaling<FastNoise::Dim::Y>(1.4f);

        CheeseCavernNoise = FastNoise::New<FastNoise::FractalFBm>();
        CheeseCavernNoise->SetSource(cheese_cavern_source);
        CheeseCavernNoise->SetOctaveCount(5);
        CheeseCavernNoise->SetLacunarity(1.8f);
        CheeseCavernNoise->SetGain(0.5f);
    }

    {
        auto spaghetti_cavern_source = FastNoise::New<FastNoise::Simplex>();
        spaghetti_cavern_source->SetScale(SPAGHETTI_CAVERN_SCALE);

        auto domain_axis_scale = FastNoise::New<FastNoise::DomainAxisScale>();
        domain_axis_scale->SetSource(spaghetti_cavern_source);
        domain_axis_scale->SetScaling<FastNoise::Dim::X>(1.f);
        domain_axis_scale->SetScaling<FastNoise::Dim::Z>(1.f);
        domain_axis_scale->SetScaling<FastNoise::Dim::Y>(1.4f);

        SpaghettiCavernNoise1 = FastNoise::New<FastNoise::FractalFBm>();
        SpaghettiCavernNoise1->SetSource(domain_axis_scale);
        SpaghettiCavernNoise1->SetLacunarity(2.4f);

        SpaghettiCavernNoise2 = FastNoise::New<FastNoise::FractalFBm>();
        SpaghettiCavernNoise2->SetSource(domain_axis_scale);
        SpaghettiCavernNoise2->SetLacunarity(2.4f);
    }
}

void TerrainGenerator::GenerateTerrain(Chunk* chunk)
{
    auto chunk_offset = FromChunkIDToChunkOffset(chunk->ID);

    // Populate noise maps
    GenerateSamples(chunk_offset);

    // Generate terrain
    for (int iz = 0; iz < WORLD_CHUNK_Z_SIZE; iz++)
    {
        for (int ix = 0; ix < WORLD_CHUNK_X_SIZE; ix++)
        {
            //const int height = static_cast<int>(std::floor(ContinentalnessSamples.At(ix, iz) * 32 + WORLD_SEA_LEVEL + 32));

            chunk->BlockData.At(ix, 0, iz) = BlockID::BEDROCK;

            for (int iy = 1; iy < WORLD_CHUNK_Y_SIZE; iy++)
            {
                Block& block = chunk->BlockData.At(ix, iy, iz);

                float cheese_sample     = CheeseCavernSamples.At(ix, iy, iz);
                float spaghetti_sample1 = SpaghettiCavernSamples1.At(ix, iy, iz);
                float spaghetti_sample2 = SpaghettiCavernSamples2.At(ix, iy, iz);

                constexpr float thickness = 0.1f;

                bool hollow = (
                    (spaghetti_sample1 < thickness && spaghetti_sample1 > -thickness) &&
                    (spaghetti_sample2 < thickness && spaghetti_sample2 > -thickness)) || cheese_sample < -0.9f;

                block = hollow ? BlockID::AIR : BlockID::STONE;
            }
        }
    }
}
