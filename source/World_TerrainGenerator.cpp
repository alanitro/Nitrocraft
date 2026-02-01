#include "World_TerrainGenerator.hpp"

#include <algorithm>
#include "World_Definitions.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"

void TerrainGenerator::Initialize(std::uint32_t world_seed)
{
    m_Rng.seed(world_seed);
}

void TerrainGenerator::GenerateTerrain(Chunk* chunk)
{
    std::uniform_int_distribution<int> dist{ -3, 3 };

    for (int z = 0; z < WORLD_CHUNK_Z_SIZE; ++z)
    {
        for (int x = 0; x < WORLD_CHUNK_X_SIZE; ++x)
        {
            int height = std::clamp(48 + dist(m_Rng), 1, 64);

            for (int y = 0; y < WORLD_CHUNK_Y_SIZE; y++)
            {
                auto& block = chunk->Blocks.At(x, y, z);

                if (y > height)           block = BlockID::AIR;
                else if (y == height)     block = BlockID::GRASS;
                else if (y >= height - 4) block = BlockID::DIRT;
                else                      block = BlockID::STONE;
            }
        }
    }
}
