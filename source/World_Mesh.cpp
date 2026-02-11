#include "World_Mesh.hpp"

#include <cstdint>
#include <array>
#include <glm/vec2.hpp>
#include "World_Coordinate.hpp"
#include "World_Chunk.hpp"

namespace
{
    constexpr std::array<std::array<float, 12>, static_cast<std::size_t>(World_Block_Face::COUNT)> BLOCK_FACES
    {
        std::array<float, 12>
        {
            0.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 0.0f,
        },
        std::array<float, 12>
        {
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f,
        },
        std::array<float, 12>
        {
            0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
        },
        std::array<float, 12>
        {
            0.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
        },
        std::array<float, 12>
        {
            1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
        },
        std::array<float, 12>
        {
            0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f,
        },
    };

    // Tilemap index
    consteval glm::vec2 TI(int s, int t)
    {
        constexpr float w = 1.0f / 16.0f;

        return glm::vec2
        (
            static_cast<float>(s) * w,
            static_cast<float>(t) * w
        );
    }

    constexpr glm::vec2 BLOCK_TILEMAP_OFFSETS[static_cast<int>(World_Block_ID::COUNT)][static_cast<int>(World_Block_Face::COUNT)]
    {
        { TI(0,0),  TI(0,0),  TI(0,0),  TI(0,0),  TI(0,0),  TI(0,0)  }, // Air == Null
        { TI(1,0),  TI(1,0),  TI(1,0),  TI(1,0),  TI(1,0),  TI(1,0)  }, // Stone
        { TI(2,0),  TI(2,0),  TI(2,0),  TI(2,0),  TI(2,0),  TI(2,0)  }, // Bedrock
        { TI(3,0),  TI(3,0),  TI(3,0),  TI(3,0),  TI(3,0),  TI(3,0)  }, // Dirt
        { TI(4,1),  TI(4,1),  TI(4,0),  TI(4,2),  TI(4,1),  TI(4,1)  }, // Grass
        { TI(5,0),  TI(5,0),  TI(5,0),  TI(5,0),  TI(5,0),  TI(5,0)  }, // Sand
        { TI(6,0),  TI(6,0),  TI(6,0),  TI(6,0),  TI(6,0),  TI(6,0)  }, // Snow
        { TI(7,0),  TI(7,0),  TI(7,0),  TI(7,0),  TI(7,0),  TI(7,0)  }, // Brick
        { TI(8,0),  TI(8,0),  TI(8,0),  TI(8,0),  TI(8,0),  TI(8,0)  }, // Glowstone
        { TI(9,1),  TI(9,1),  TI(9,0),  TI(9,2),  TI(9,1),  TI(9,1)  }, // Oak
        { TI(10,0), TI(10,0), TI(10,0), TI(10,0), TI(10,0), TI(10,0) }, // Oak Leaves
        { TI(11,0), TI(11,0), TI(11,0), TI(11,0), TI(11,0), TI(11,0) }, // Oak Wood
    };
}


void World_Mesh_GenerateChunkCPUMesh(
    const World_Chunk* chunk,
    std::vector<World_Mesh_ChunkCPUMeshVertex>& vertices,
    std::vector<std::uint32_t>& indices
)
{
    World_GlobalXYZ chunk_offset = World_FromChunkIDToChunkOffset(chunk->ID);

    for (int z = 0; z < World_CHUNK_Z_SIZE; z++)
    {
        for (int x = 0; x < World_CHUNK_X_SIZE; x++)
        {
            for (int y = 0; y < World_CHUNK_Y_SIZE; y++)
            {
                // Block face detection
                World_Block block = chunk->GetBlockAt(World_LocalXYZ(x, y, z));

                if (block.ID == World_Block_ID::AIR) continue;

                auto neighbour_blocks = chunk->GetNeighbourBlocksAt(World_LocalXYZ(x, y, z));

                std::uint32_t blockface_bitmask = 0;

                for (std::size_t face = (std::size_t)World_Block_Face::XN; face <= (std::size_t)World_Block_Face::ZP; face++)
                {
                    if (neighbour_blocks[face].IsTransparent()) blockface_bitmask |= (1u << (std::uint32_t)face);
                }

                if (blockface_bitmask == 0) continue;

                // Chunk Mesh generation
                World_GlobalXYZ block_offset = chunk_offset + World_GlobalXYZ(x, y, z);

                auto neighbour_lights = chunk->GetNeighbourLightsAt(World_LocalXYZ(x, y, z));

                for (std::size_t face = (std::size_t)World_Block_Face::XN; face <= (std::size_t)World_Block_Face::ZP; face++)
                {
                    if (!(blockface_bitmask & (1u << face))) continue;

                    // Populate vertices 
                    const auto& block_face = BLOCK_FACES[(std::size_t)face];

                    glm::vec2 tile_map_offset = BLOCK_TILEMAP_OFFSETS[(std::size_t)block.ID][(std::size_t)face];

                    for (int i = 0; i < 4; i++)
                    {
                        int vertex_base = i * 3;

                        constexpr float w = 1.0f / 16.0f;

                        vertices.emplace_back(
                            block_face[vertex_base + 0] + block_offset.x,
                            block_face[vertex_base + 1] + block_offset.y,
                            block_face[vertex_base + 2] + block_offset.z,
                            tile_map_offset.x + ((i == 1 || i == 2) ? w : 0.0f),
                            tile_map_offset.y + ((i == 2 || i == 3) ? w : 0.0f),
                            static_cast<std::uint8_t>(face),
                            static_cast<std::uint8_t>(neighbour_lights[face])
                        );
                    }

                    // Populate indices
                    std::uint32_t base_index = static_cast<std::uint32_t>(vertices.size() - 4);
                    indices.insert(
                        indices.end(),
                        {
                            base_index + 0, base_index + 1, base_index + 2,
                            base_index + 0, base_index + 2, base_index + 3
                        }
                    );
                }
            }
        }
    }
}
