#include "Graphics_Mesh.hpp"

#include <cstdint>
#include <array>
#include <glm/vec2.hpp>
#include "World_Coordinate.hpp"
#include "World_Chunk.hpp"

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

void Graphics_Mesh_GenerateChunkCPUMesh(const World_Chunk* chunk, Graphics_ChunkCPUMesh& mesh)
{
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

                mesh.Vertices.emplace_back(
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
            std::uint32_t base_index = static_cast<std::uint32_t>(mesh.Vertices.size() - 4);
            mesh.Indices.insert(
                mesh.Indices.end(),
                {
                    base_index + 0, base_index + 1, base_index + 2,
                    base_index + 0, base_index + 2, base_index + 3
                }
            );
        }
    }
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

void Graphics_Mesh_GenerateChunkCPUMesh_AmbientOcclusion(const World_Chunk* chunk, Graphics_ChunkCPUMesh& mesh)
{
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

                mesh.Vertices.emplace_back(
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
            std::uint32_t base_index = static_cast<std::uint32_t>(mesh.Vertices.size() - 4);

            if (ao_states[1] + ao_states[3] <= ao_states[0] + ao_states[2])
            {
                mesh.Indices.insert(
                    mesh.Indices.end(),
                    {
                        base_index + 0, base_index + 1, base_index + 2,
                        base_index + 0, base_index + 2, base_index + 3,
                    }
                );
            }
            else
            {
                mesh.Indices.insert(
                    mesh.Indices.end(),
                    {
                        base_index + 0, base_index + 1, base_index + 3,
                        base_index + 1, base_index + 2, base_index + 3,
                    }
                );
            }
        }
    }
}

Graphics_ChunkGPUMeshHandle::Graphics_ChunkGPUMeshHandle(std::uint32_t storage_version)
{
    Version = storage_version;

    glGenVertexArrays(1, &VertexArrayID);
    glGenBuffers(1, &VertexBufferID);
    glGenBuffers(1, &IndexBufferID);

    glBindVertexArray(VertexArrayID);
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Graphics_ChunkCPUMeshVertexLayout), reinterpret_cast<const void*>(offsetof(Graphics_ChunkCPUMeshVertexLayout, X)));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Graphics_ChunkCPUMeshVertexLayout), reinterpret_cast<const void*>(offsetof(Graphics_ChunkCPUMeshVertexLayout, S)));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(Graphics_ChunkCPUMeshVertexLayout), reinterpret_cast<const void*>(offsetof(Graphics_ChunkCPUMeshVertexLayout, F)));
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, sizeof(Graphics_ChunkCPUMeshVertexLayout), reinterpret_cast<const void*>(offsetof(Graphics_ChunkCPUMeshVertexLayout, L)));
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_BYTE, sizeof(Graphics_ChunkCPUMeshVertexLayout), reinterpret_cast<const void*>(offsetof(Graphics_ChunkCPUMeshVertexLayout, AO)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    IndicesCount = 0;

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

Graphics_ChunkGPUMeshHandle::~Graphics_ChunkGPUMeshHandle()
{
    glDeleteVertexArrays(1, &VertexArrayID);
    glDeleteBuffers(1, &VertexBufferID);
    glDeleteBuffers(1, &IndexBufferID);
}

void Graphics_ChunkGPUMeshHandle::UploadCPUMeshToGPU(const Graphics_ChunkCPUMesh& cpumesh)
{
    glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);

    // TODO: use buffer orphaning instead of recreating new buffer
    glBufferData(GL_ARRAY_BUFFER, cpumesh.Vertices.size() * sizeof(Graphics_ChunkCPUMeshVertexLayout), cpumesh.Vertices.data(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cpumesh.Indices.size() * sizeof(std::uint32_t), cpumesh.Indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    IndicesCount = static_cast<std::uint32_t>(cpumesh.Indices.size());
}
