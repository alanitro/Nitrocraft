#include "World_Generation.hpp"

#include <algorithm>
#include <FastNoise/FastNoise.h>
#include "Utility_Array2D.hpp"
#include "Utility_Array3D.hpp"
#include "World_Coordinate.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"

namespace
{
    constexpr float CONTINENTALNESS_SCALE  = 625.0f;
    constexpr float CHEESE_CAVERN_SCALE    = 200.0f;
    constexpr float SPAGHETTI_CAVERN_SCALE = 200.0f;

    constexpr std::size_t SAMPLE_X_SIZE = World_CHUNK_X_SIZE;
    constexpr std::size_t SAMPLE_Y_SIZE = World_CHUNK_Y_SIZE;
    constexpr std::size_t SAMPLE_Z_SIZE = World_CHUNK_Z_SIZE;

    int GenerationSeed;

    // Terrain noise
    thread_local FastNoise::SmartNode<FastNoise::FractalFBm>  ContinentalnessNoise{};

    // Cavern noise
    thread_local FastNoise::SmartNode<FastNoise::FractalFBm>  CheeseCavernNoise{};
    thread_local FastNoise::SmartNode<FastNoise::FractalFBm>  SpaghettiCavernNoise1{};
    thread_local FastNoise::SmartNode<FastNoise::FractalFBm>  SpaghettiCavernNoise2{};

    // Noise sample storage
    thread_local Array2D<float, SAMPLE_X_SIZE, SAMPLE_Z_SIZE>                ContinentalnessSamples;
    thread_local Array3D<float, SAMPLE_X_SIZE, SAMPLE_Y_SIZE, SAMPLE_Z_SIZE> CheeseCavernSamples;
    thread_local Array3D<float, SAMPLE_X_SIZE, SAMPLE_Y_SIZE, SAMPLE_Z_SIZE> SpaghettiCavernSamples1;
    thread_local Array3D<float, SAMPLE_X_SIZE, SAMPLE_Y_SIZE, SAMPLE_Z_SIZE> SpaghettiCavernSamples2;

    void GenerateSamples(World_GlobalXYZ chunk_offset)
    {
        float cx = static_cast<float>(chunk_offset.x);
        float cy = static_cast<float>(chunk_offset.y);
        float cz = static_cast<float>(chunk_offset.z);

        ContinentalnessNoise->GenUniformGrid2D(
            ContinentalnessSamples.Data(),
            cx, cz,
            SAMPLE_X_SIZE, SAMPLE_Z_SIZE,
            1.0f, 1.0f,
            GenerationSeed
        );

        CheeseCavernNoise->GenUniformGrid3D(
            CheeseCavernSamples.Data(),
            cx, cy, cz,
            SAMPLE_X_SIZE, SAMPLE_Y_SIZE, SAMPLE_Z_SIZE,
            1.0f, 1.0f, 1.0f,
            GenerationSeed
        );

        SpaghettiCavernNoise1->GenUniformGrid3D(
            SpaghettiCavernSamples1.Data(),
            cx, cy, cz,
            SAMPLE_X_SIZE, SAMPLE_Y_SIZE, SAMPLE_Z_SIZE,
            1.0f, 1.0f, 1.0f,
            GenerationSeed + 10000
        );

        SpaghettiCavernNoise2->GenUniformGrid3D(
            SpaghettiCavernSamples2.Data(),
            cx, cy, cz,
            SAMPLE_X_SIZE, SAMPLE_Y_SIZE, SAMPLE_Z_SIZE,
            1.0f, 1.0f, 1.0f,
            GenerationSeed + 20000
        );
    }
}

void World_Generation_Initialize(int generation_seed)
{
    GenerationSeed = generation_seed;

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
        domain_axis_scale->SetScaling<FastNoise::Dim::X>(0.8f);
        domain_axis_scale->SetScaling<FastNoise::Dim::Z>(0.8f);
        domain_axis_scale->SetScaling<FastNoise::Dim::Y>(1.4f);

        CheeseCavernNoise = FastNoise::New<FastNoise::FractalFBm>();
        CheeseCavernNoise->SetSource(cheese_cavern_source);
        CheeseCavernNoise->SetOctaveCount(5);
        CheeseCavernNoise->SetLacunarity(2.2f);
        CheeseCavernNoise->SetGain(0.5f);
    }

    {
        auto spaghetti_cavern_source = FastNoise::New<FastNoise::Simplex>();
        spaghetti_cavern_source->SetScale(SPAGHETTI_CAVERN_SCALE);

        auto domain_axis_scale = FastNoise::New<FastNoise::DomainAxisScale>();
        domain_axis_scale->SetSource(spaghetti_cavern_source);
        domain_axis_scale->SetScaling<FastNoise::Dim::X>(0.8f);
        domain_axis_scale->SetScaling<FastNoise::Dim::Z>(0.8f);
        domain_axis_scale->SetScaling<FastNoise::Dim::Y>(1.2f);

        SpaghettiCavernNoise1 = FastNoise::New<FastNoise::FractalFBm>();
        SpaghettiCavernNoise1->SetSource(domain_axis_scale);
        CheeseCavernNoise->SetOctaveCount(4);
        SpaghettiCavernNoise1->SetLacunarity(2.4f);

        SpaghettiCavernNoise2 = FastNoise::New<FastNoise::FractalFBm>();
        SpaghettiCavernNoise2->SetSource(domain_axis_scale);
        CheeseCavernNoise->SetOctaveCount(4);
        SpaghettiCavernNoise2->SetLacunarity(2.4f);
    }
}

void World_Generation_GenerateChunk(World_Chunk* chunk)
{
    auto chunk_offset = World_FromChunkIDToChunkOffset(chunk->ID);

    // Populate noise maps
    GenerateSamples(chunk_offset);

    // Populate block data
    for (int iz = 0; iz < World_CHUNK_Z_SIZE; iz++)
    {
        for (int ix = 0; ix < World_CHUNK_X_SIZE; ix++)
        {
            const int height = static_cast<int>(std::floor(ContinentalnessSamples.At(ix, iz) * 64 + World_SEA_LEVEL + 64));

            chunk->Storage->Blocks.At(ix, 0, iz).ID = World_Block_ID::BEDROCK;

            for (int iy = 1; iy < World_CHUNK_Y_SIZE; iy++)
            {
                auto& block = chunk->Storage->Blocks.At(ix, iy, iz);

                float cheese_sample     = CheeseCavernSamples.At(ix, iy, iz);
                float spaghetti_sample1 = SpaghettiCavernSamples1.At(ix, iy, iz);
                float spaghetti_sample2 = SpaghettiCavernSamples2.At(ix, iy, iz);

                constexpr float thickness = 0.085f;

                float density = static_cast<float>(iy) / static_cast<float>(height);

                bool hollow = (
                    (spaghetti_sample1 < thickness && spaghetti_sample1 > -thickness) &&
                    (spaghetti_sample2 < thickness && spaghetti_sample2 > -thickness)) || cheese_sample < (-0.65f - density);

                if (iy < height && !hollow)         block.ID = World_Block_ID::STONE;
                else if (iy == height && !hollow)   block.ID = World_Block_ID::GRASS;
                else                                block.ID = World_Block_ID::AIR;
            }
        }
    }

    // Populate height data
    for (int iz = 0; iz < World_CHUNK_Z_SIZE; iz++)
    {
        for (int ix = 0; ix < World_CHUNK_X_SIZE; ix++)
        {
            for (int iy = World_CHUNK_Y_SIZE - 1; iy >= 0; iy--)
            {
                if (chunk->Storage->Blocks.At(ix, iy, iz).ID != World_Block_ID::AIR)
                {
                    chunk->Storage->Heights.At(ix, iz) = static_cast<std::uint8_t>(iy);

                    break;
                }
            }
        }
    }
}
