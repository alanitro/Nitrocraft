#include "World_TerrainGenerator.hpp"

namespace nitrocraft::world
{

void nitrocraft::world::TerrainGenerator::Initialize(int generation_seed)
{
    m_generation_seed = generation_seed;

    constexpr float continentalness_scale = 1400.0f;
    constexpr float cheese_cavern_scale = 220.0f;
    constexpr float spaghetti_cavern_scale = 230.0f;

    {
        auto continentalness_source = FastNoise::New<FastNoise::SuperSimplex>();
        continentalness_source->SetScale(continentalness_scale);

        m_continentalness_noise = FastNoise::New<FastNoise::FractalFBm>();
        m_continentalness_noise->SetSource(continentalness_source);
        m_continentalness_noise->SetOctaveCount(5);
        m_continentalness_noise->SetLacunarity(2.6f);
        m_continentalness_noise->SetGain(0.5f);
    }

    {
        auto cheese_cavern_source = FastNoise::New<FastNoise::Simplex>();
        cheese_cavern_source->SetScale(cheese_cavern_scale);

        auto domain_axis_scale = FastNoise::New<FastNoise::DomainAxisScale>();
        domain_axis_scale->SetSource(cheese_cavern_source);
        domain_axis_scale->SetScaling<FastNoise::Dim::X>(0.8f);
        domain_axis_scale->SetScaling<FastNoise::Dim::Z>(0.8f);
        domain_axis_scale->SetScaling<FastNoise::Dim::Y>(1.4f);


        m_cheese_noise = FastNoise::New<FastNoise::FractalFBm>();
        m_cheese_noise->SetSource(cheese_cavern_source);
        m_cheese_noise->SetOctaveCount(4);
        m_cheese_noise->SetLacunarity(2.2f);
        m_cheese_noise->SetGain(0.5f);
    }

    {
        auto spaghetti_cavern_source = FastNoise::New<FastNoise::Simplex>();
        spaghetti_cavern_source->SetScale(spaghetti_cavern_scale);

        auto domain_axis_scale = FastNoise::New<FastNoise::DomainAxisScale>();
        domain_axis_scale->SetSource(spaghetti_cavern_source);
        domain_axis_scale->SetScaling<FastNoise::Dim::X>(0.8f);
        domain_axis_scale->SetScaling<FastNoise::Dim::Z>(0.8f);
        domain_axis_scale->SetScaling<FastNoise::Dim::Y>(1.2f);

        m_spaghetti_noise1 = FastNoise::New<FastNoise::FractalFBm>();
        m_spaghetti_noise1->SetSource(domain_axis_scale);
        m_spaghetti_noise1->SetOctaveCount(3);
        m_spaghetti_noise1->SetLacunarity(2.4f);

        m_spaghetti_noise2 = FastNoise::New<FastNoise::FractalFBm>();
        m_spaghetti_noise2->SetSource(domain_axis_scale);
        m_spaghetti_noise2->SetOctaveCount(3);
        m_spaghetti_noise2->SetLacunarity(2.4f);
    }
}



void nitrocraft::world::TerrainGenerator::GenerateTerrain(Chunk* chunk)
{
    thread_local SampleBuffers sample_buffers;

    PopulateSamples(FromChunkIDToChunkOffset(chunk->id), sample_buffers);

    PopulateBlocks(chunk, sample_buffers);

    PopulateHeights(chunk);
}

void TerrainGenerator::PopulateSamples(GlobalXYZ chunk_offset, SampleBuffers& samples)
{
    float cx = static_cast<float>(chunk_offset.x);
    float cy = static_cast<float>(chunk_offset.y);
    float cz = static_cast<float>(chunk_offset.z);

    m_continentalness_noise->GenUniformGrid2D(
        samples.continentalness.Data(),
        cx, cz,
        CHUNK_X_SIZE, CHUNK_Z_SIZE,
        1.0f, 1.0f,
        m_generation_seed
    );

    m_cheese_noise->GenUniformGrid3D(
        samples.cheese_cavern.Data(),
        cx, cy, cz,
        CHUNK_X_SIZE, CHUNK_Y_SIZE, CHUNK_Z_SIZE,
        1.0f, 1.0f, 1.0f,
        m_generation_seed
    );

    m_spaghetti_noise1->GenUniformGrid3D(
        samples.spaghetti_cavern1.Data(),
        cx, cy, cz,
        CHUNK_X_SIZE, CHUNK_Y_SIZE, CHUNK_Z_SIZE,
        1.0f, 1.0f, 1.0f,
        m_generation_seed + 10000
    );

    m_spaghetti_noise2->GenUniformGrid3D(
        samples.spaghetti_cavern2.Data(),
        cx, cy, cz,
        CHUNK_X_SIZE, CHUNK_Y_SIZE, CHUNK_Z_SIZE,
        1.0f, 1.0f, 1.0f,
        m_generation_seed + 20000
    );
}

void TerrainGenerator::PopulateBlocks(Chunk * chunk, SampleBuffers& samples)
{
    for (int iz = 0; iz < CHUNK_Z_SIZE; iz++)
    for (int ix = 0; ix < CHUNK_X_SIZE; ix++)
    {
        const int height = static_cast<int>(std::floor(samples.continentalness.At(ix, iz) * 64 + SEA_LEVEL + 64));

        chunk->storage->blocks.At(ix, 0, iz).id = BlockID::BEDROCK;

        for (int iy = 1; iy < CHUNK_Y_SIZE; iy++)
        {
            auto& block = chunk->storage->blocks.At(ix, iy, iz);

            float cheese_sample = samples.cheese_cavern.At(ix, iy, iz);
            float spaghetti_sample1 = samples.spaghetti_cavern1.At(ix, iy, iz);
            float spaghetti_sample2 = samples.spaghetti_cavern2.At(ix, iy, iz);

            constexpr float thickness = 0.085f;

            float density = static_cast<float>(iy) / static_cast<float>(height);

            bool spaghetti_hollow1 = spaghetti_sample1 < thickness && spaghetti_sample1 > -thickness;
            bool spaghetti_hollow2 = spaghetti_sample2 < thickness && spaghetti_sample2 > -thickness;
            bool hollow = (spaghetti_hollow1 && spaghetti_hollow2) || cheese_sample < (-0.65f - density);

            if (iy < height && !hollow)         block.id = BlockID::STONE;
            else if (iy == height && !hollow)   block.id = BlockID::GRASS;
            else                                block.id = BlockID::AIR;
        }
    }
}

void TerrainGenerator::PopulateHeights(Chunk * chunk)
{
    for (int iz = 0; iz < CHUNK_Z_SIZE; iz++)
    for (int ix = 0; ix < CHUNK_X_SIZE; ix++)
    for (int iy = CHUNK_Y_SIZE - 1; iy >= 0; iy--)
    {
        if (chunk->storage->blocks.At(ix, iy, iz).id != BlockID::AIR)
        {
            chunk->storage->heights.At(ix, iz) = static_cast<std::uint8_t>(iy);

            break;
        }
    }
}

} // namespace nitrocraft::world
