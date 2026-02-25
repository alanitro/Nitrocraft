#include "Graphics_Mesh.hpp"

#include <cstdint>
#include <array>
#include <glm/vec2.hpp>
#include "World_Coordinate.hpp"
#include "World_Chunk.hpp"
#include <print>//TODO:remove

namespace
{
    // Front face quad vertices are laid out in counter clock wise order.
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

Graphics_ChunkCPUMesh Graphics_Mesh_GenerateChunkCPUMesh(const World_Chunk* chunk)
{
    Graphics_ChunkCPUMesh cpumesh{ const_cast<World_Chunk*>(chunk), chunk->StorageVersion.load(std::memory_order_acquire) };

    World_GlobalXYZ chunk_offset = World_FromChunkIDToChunkOffset(chunk->ID);

    for (int lz = 0; lz < World_CHUNK_Z_SIZE; lz++)
    for (int lx = 0; lx < World_CHUNK_X_SIZE; lx++)
    for (int ly = 0; ly < World_CHUNK_Y_SIZE; ly++)
    {
        // Block face detection
        World_Block block = chunk->GetBlockAt(World_LocalXYZ(lx, ly, lz));

        if (block.ID == World_Block_ID::AIR) continue;

        auto neighbour_blocks = chunk->GetWholeNeighbourBlocksAt(World_LocalXYZ(lx, ly, lz));

        std::uint32_t blockface_bitmask = 0;

        for (std::size_t face = (std::size_t)World_Block_Face::XN; face <= (std::size_t)World_Block_Face::ZP; face++)
        {
            if (neighbour_blocks[face].IsTransparent()) blockface_bitmask |= (1u << (std::uint32_t)face);
        }

        if (blockface_bitmask == 0) continue;

        // Chunk Mesh generation
        World_GlobalXYZ block_offset = chunk_offset + World_GlobalXYZ(lx, ly, lz);

        auto neighbour_lights = chunk->GetCrossNeighbourLightsAt(World_LocalXYZ(lx, ly, lz));

        for (std::size_t face = (std::size_t)World_Block_Face::XN; face <= (std::size_t)World_Block_Face::ZP; face++)
        {
            if (!(blockface_bitmask & (1u << face))) continue;

            // Populate vertices 
            const auto& block_face = BLOCK_FACES[(std::size_t)face];

            glm::vec2 tile_map_offset = BLOCK_TILEMAP_OFFSETS[(std::size_t)block.ID][(std::size_t)face];

            for (int vi = 0; vi < 4; vi++)
            {
                int vertex_base = vi * 3;

                constexpr float w = 1.0f / 16.0f;

                cpumesh.Vertices.emplace_back(
                    block_face[vertex_base + 0] + block_offset.x,
                    block_face[vertex_base + 1] + block_offset.y,
                    block_face[vertex_base + 2] + block_offset.z,
                    tile_map_offset.x + ((vi == 1 || vi == 2) ? w : 0.0f),
                    tile_map_offset.y + ((vi == 2 || vi == 3) ? w : 0.0f),
                    static_cast<std::uint8_t>(face),
                    static_cast<std::uint8_t>(neighbour_lights[face])
                );
            }

            // Populate indices
            std::uint32_t base_index = static_cast<std::uint32_t>(cpumesh.Vertices.size() - 4);
            cpumesh.Indices.insert(
                cpumesh.Indices.end(),
                {
                    base_index + 0, base_index + 1, base_index + 2,
                    base_index + 0, base_index + 2, base_index + 3
                }
            );
        }
    }

    return cpumesh;
}

namespace
{
    // AO Values
    float AOValues[] = { 0.1f, 0.25f, 0.5f, 1.0f };

    // [0,4): quad vertex index. 0 -> 1 -> 2 and 0 -> 2 -> 3 (front face quad counter clockwise)
    // 3 --- 2
    // |     |    <- front face
    // 0 --- 1
    // [0, 3): 0==side1, 1==side2, 2==corner
    constexpr int NeighbourBlockIndicesPerFaceVertex[(std::size_t)World_Block_Face::COUNT][4][3] =
    {
        // Face XN
        {
            { 6, 14, 18 }, // { non, nno, nnn }
            { 14, 8, 22 }, // { nno, nop, nnp }
            { 8, 16, 24 }, // { nop, npo, npp }
            { 6, 16, 20 }, // { non, npo, npn }
        },

        // Face XP
        {
            { 9, 15, 23 }, // { pop, pno, pnp }
            { 7, 15, 19 }, // { pon, pno, pnn }
            { 7, 17, 21 }, // { pon, ppo, ppn }
            { 9, 17, 25 }, // { pop, ppo, ppp }
        },

        // Face YN
        {
            { 14, 10, 18 }, // { nno, onn, nnn }
            { 15, 10, 19 }, // { pno, onn, pnn }
            { 15, 12, 23 }, // { pno, onp, pnp }
            { 14, 12, 22 }, // { nno, onp, nnp }
        },

        // Face YP
        {
            { 16, 13, 24 }, // { npo, opp, npp }
            { 17, 13, 25 }, // { ppo, opp, ppp }
            { 17, 11, 21 }, // { ppo, opn, ppn }
            { 16, 11, 20 }, // { npo, opn, npn }
        },

        // Face ZN
        {
            { 7, 10, 19 }, // { pon, onn, pnn }
            { 6, 10, 18 }, // { non, onn, nnn }
            { 6, 11, 20 }, // { non, opn, npn }
            { 7, 11, 21 }, // { pon, opn, ppn }
        },

        // Face ZP
        {
            { 8, 12, 22 }, // { nop, onp, nnp }
            { 9, 12, 23 }, // { pop, onp, pnp }
            { 9, 13, 25 }, // { pop, opp, ppp }
            { 8, 13, 24 }, // { nop, opp, npp }
        },
    };

    constexpr int GetAOState(int side1, int side2, int corner)
    {
        if (side1 && side2) return 0;

        return 3 - (side1 + side2 + corner);
    }
}

Graphics_ChunkCPUMesh Graphics_Mesh_GenerateChunkCPUMesh_AmbientOcclusion(const World_Chunk* chunk)
{
    Graphics_ChunkCPUMesh cpumesh{ const_cast<World_Chunk*>(chunk) };

    World_GlobalXYZ chunk_offset = World_FromChunkIDToChunkOffset(chunk->ID);

    for (int lz = 0; lz < World_CHUNK_Z_SIZE; lz++)
    for (int lx = 0; lx < World_CHUNK_X_SIZE; lx++)
    for (int ly = 0; ly < World_CHUNK_Y_SIZE; ly++)
    {
        // Block face detection
        World_Block block = chunk->GetBlockAt(World_LocalXYZ(lx, ly, lz));

        if (block.ID == World_Block_ID::AIR) continue;

        auto neighbour_blocks = chunk->GetWholeNeighbourBlocksAt(World_LocalXYZ(lx, ly, lz));

        std::uint32_t blockface_bitmask = 0;

        for (std::size_t face = (std::size_t)World_Block_Face::XN; face <= (std::size_t)World_Block_Face::ZP; face++)
        {
            if (neighbour_blocks[face].IsTransparent()) blockface_bitmask |= (1u << (std::uint32_t)face);
        }

        if (blockface_bitmask == 0) continue;

        // Chunk mesh generation
        World_GlobalXYZ block_offset = chunk_offset + World_GlobalXYZ(lx, ly, lz);

        auto neighbour_lights = chunk->GetCrossNeighbourLightsAt(World_LocalXYZ(lx, ly, lz));

        for (std::size_t face = (std::size_t)World_Block_Face::XN; face <= (std::size_t)World_Block_Face::ZP; face++)
        {
            if (!(blockface_bitmask & (1u << face))) continue;

            // Populate vertices 
            const auto& block_face = BLOCK_FACES[(std::size_t)face];

            glm::vec2 tile_map_offset = BLOCK_TILEMAP_OFFSETS[(std::size_t)block.ID][(std::size_t)face];

            int ao_states[4];

            for (int vi = 0; vi < 4; vi++)
            {
                int vertex_base = vi * 3;

                constexpr float w = 1.0f / 16.0f;

                int side1_block_index  = NeighbourBlockIndicesPerFaceVertex[face][vi][0];
                int side2_block_index  = NeighbourBlockIndicesPerFaceVertex[face][vi][1];
                int corner_block_index = NeighbourBlockIndicesPerFaceVertex[face][vi][2];

                World_Block side1  = neighbour_blocks[side1_block_index];
                World_Block side2  = neighbour_blocks[side2_block_index];
                World_Block corner = neighbour_blocks[corner_block_index];

                ao_states[vi] = GetAOState(
                    side1.IsOpaque()  ? 1 : 0,
                    side2.IsOpaque()  ? 1 : 0,
                    corner.IsOpaque() ? 1 : 0
                );

                cpumesh.Vertices.emplace_back(
                    block_face[vertex_base + 0] + block_offset.x,
                    block_face[vertex_base + 1] + block_offset.y,
                    block_face[vertex_base + 2] + block_offset.z,
                    tile_map_offset.x + ((vi == 1 || vi == 2) ? w : 0.0f),
                    tile_map_offset.y + ((vi == 2 || vi == 3) ? w : 0.0f),
                    static_cast<std::uint8_t>(face),
                    static_cast<std::uint8_t>(neighbour_lights[face]),
                    static_cast<std::uint8_t>(ao_states[vi])
                );
            }

            // Populate indices
            std::uint32_t base_index = static_cast<std::uint32_t>(cpumesh.Vertices.size() - 4);

            if (ao_states[1] + ao_states[3] <= ao_states[0] + ao_states[2])
            {
                cpumesh.Indices.insert(
                    cpumesh.Indices.end(),
                    {
                        base_index + 0, base_index + 1, base_index + 2,
                        base_index + 0, base_index + 2, base_index + 3,
                    }
                );
            }
            else
            {
                cpumesh.Indices.insert(
                    cpumesh.Indices.end(),
                    {
                        base_index + 0, base_index + 1, base_index + 3,
                        base_index + 1, base_index + 2, base_index + 3,
                    }
                );
            }
        }
    }

    return cpumesh;
}

Graphics_ChunkGPUMeshHandle::Graphics_ChunkGPUMeshHandle()
{
    glGenVertexArrays(1, &VertexArrayID);
    glGenBuffers(1, &VertexBufferID);
    glGenBuffers(1, &IndexBufferID);

    glBindVertexArray(VertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Graphics_ChunkMeshVertexLayout), reinterpret_cast<const void*>(offsetof(Graphics_ChunkMeshVertexLayout, X)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Graphics_ChunkMeshVertexLayout), reinterpret_cast<const void*>(offsetof(Graphics_ChunkMeshVertexLayout, S)));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(Graphics_ChunkMeshVertexLayout), reinterpret_cast<const void*>(offsetof(Graphics_ChunkMeshVertexLayout, F)));
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, sizeof(Graphics_ChunkMeshVertexLayout), reinterpret_cast<const void*>(offsetof(Graphics_ChunkMeshVertexLayout, L)));
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_BYTE, sizeof(Graphics_ChunkMeshVertexLayout), reinterpret_cast<const void*>(offsetof(Graphics_ChunkMeshVertexLayout, AO)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    IndicesCount = 0;

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    static int counter = 0;
    //std::println("Created {}", ++counter); //TODO:remove
}

Graphics_ChunkGPUMeshHandle::~Graphics_ChunkGPUMeshHandle()
{
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteBuffers(1, &VertexBufferID);
    glDeleteBuffers(1, &IndexBufferID);
    //std::println("Destroyed"); //TODO:remove
}
