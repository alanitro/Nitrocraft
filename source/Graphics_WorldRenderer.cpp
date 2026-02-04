#include "Graphics_WorldRenderer.hpp"

#include <array>
#include <unordered_map>
#include <print>
#include <glm/vec2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>
#include "Utility_IO.hpp"
#include "Graphics_Camera.hpp"
#include "World_Definitions.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"

namespace
{
    struct ChunkMeshVertex
    {
        float        X; // Vertex position (x,y,z)
        float        Y;
        float        Z;
        float        S; // Texture coordinate (s,t)
        float        T;
        std::uint8_t F; // Face
    };

    struct ChunkMesh
    {
        GLuint VertexArrayID;
        GLuint VertexBufferID;
        GLuint IndexBufferID;
        std::uint32_t IndicesCount;

        void Create();
        void Destroy();
    };

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

    GLuint i_ShaderProgram = 0;
    GLuint i_Texture = 0;

    std::vector<ChunkMesh*>                 i_ChunksToRender;
    std::unordered_map<World_ChunkID, ChunkMesh>  i_ChunkMeshes;

    void PushChunksToRender(const World_Chunk* chunk);
    void GenerateMesh(std::vector<ChunkMeshVertex>& vertices, std::vector<std::uint32_t>& indices, const World_Chunk* chunk);
    void LoadShaderProgram();
    void LoadTexture();

} // !namespace internal

void WorldRenderer_Initialize()
{
    LoadShaderProgram();

    LoadTexture();
}

void WorldRenderer_Terminate()
{
    for (auto& [chunk_id, chunk_mesh] : i_ChunkMeshes)
    {
        chunk_mesh.Destroy();
    }

    glDeleteProgram(i_ShaderProgram);

    glDeleteTextures(1, &i_Texture);
}

void WorldRenderer_Render(const Camera& camera)
{
    glUseProgram(i_ShaderProgram);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, i_Texture);
    glUniform1i(glGetUniformLocation(i_ShaderProgram, "u_Texture"), 0);

    auto& model_view_projection = camera.GetViewProjection();

    glUniformMatrix4fv(glGetUniformLocation(i_ShaderProgram, "u_ModelViewProjection"), 1, GL_FALSE, glm::value_ptr(model_view_projection));

    for (auto chunk_mesh : i_ChunksToRender)
    {
        glBindVertexArray(chunk_mesh->VertexArrayID);

        glDrawElements(GL_TRIANGLES, chunk_mesh->IndicesCount, GL_UNSIGNED_INT, reinterpret_cast<const void*>(0));
    }

    i_ChunksToRender.clear();
}

void WorldRenderer_PrepareChunksToRender(const Array2D<World_Chunk*, World_LOADING_DIAMETER, World_LOADING_DIAMETER>& active_area)
{
    constexpr int diff = World_LOADING_RADIUS - World_RENDER_DISTANCE;

    for (int ix = diff; ix < World_LOADING_DIAMETER - diff; ix++)
    {
        for (int iz = diff; iz < World_LOADING_DIAMETER - diff; iz++)
        {
            PushChunksToRender(active_area.At(ix, iz));
        }
    }
}

namespace // internal
{
    void PushChunksToRender(const World_Chunk* chunk)
    {
        World_ChunkID chunk_id = chunk->ID;
        World_GlobalXYZ chunk_offset = World_FromChunkIDToChunkOffset(chunk->ID);

        if (const auto& iter = i_ChunkMeshes.find(chunk_id); iter != i_ChunkMeshes.end())
        {
            i_ChunksToRender.push_back(&iter->second);
            return;
        }
        else
        {
            iter->second.Destroy();
            i_ChunkMeshes.erase(chunk_id);
        }

        auto [it, inserted] = i_ChunkMeshes.emplace(chunk_id, ChunkMesh{});

        auto& mesh = it->second;
        mesh.Create();

        glBindVertexArray(mesh.VertexArrayID);
        glBindBuffer(GL_ARRAY_BUFFER, mesh.VertexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.IndexBufferID);

        std::vector<ChunkMeshVertex> vertices;
        std::vector<std::uint32_t>   indices;

        GenerateMesh(vertices, indices, chunk);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(ChunkMeshVertex), vertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(std::uint32_t), indices.data(), GL_STATIC_DRAW);

        mesh.IndicesCount = static_cast<std::uint32_t>(indices.size());

        i_ChunksToRender.push_back(&mesh);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void GenerateMesh(std::vector<ChunkMeshVertex>& vertices, std::vector<std::uint32_t>& indices, const World_Chunk* chunk)
    {
        World_GlobalXYZ chunk_offset = World_FromChunkIDToChunkOffset(chunk->ID);

        for (int z = 0; z < World_CHUNK_Z_SIZE; z++)
        {
            for (int x = 0; x < World_CHUNK_X_SIZE; x++)
            {
                for (int y = 0; y < World_CHUNK_Y_SIZE; y++)
                {
                    // Block face detection
                    World_Block block = World_Chunk_GetBlockAt(chunk, { x, y, z });

                    if (block.ID == World_Block_ID::AIR) continue;

                    auto neighbour_blocks = World_Chunk_GetNeighbourBlocks(chunk, { x, y, z });

                    std::uint32_t blockface_bitmask = 0;

                    if (World_Block_IsTransparent(neighbour_blocks[static_cast<int>(World_Block_Face::XN)])) blockface_bitmask |= static_cast<std::uint32_t>(World_Block_FaceBit::XN);
                    if (World_Block_IsTransparent(neighbour_blocks[static_cast<int>(World_Block_Face::XP)])) blockface_bitmask |= static_cast<std::uint32_t>(World_Block_FaceBit::XP);
                    if (World_Block_IsTransparent(neighbour_blocks[static_cast<int>(World_Block_Face::YN)])) blockface_bitmask |= static_cast<std::uint32_t>(World_Block_FaceBit::YN);
                    if (World_Block_IsTransparent(neighbour_blocks[static_cast<int>(World_Block_Face::YP)])) blockface_bitmask |= static_cast<std::uint32_t>(World_Block_FaceBit::YP);
                    if (World_Block_IsTransparent(neighbour_blocks[static_cast<int>(World_Block_Face::ZN)])) blockface_bitmask |= static_cast<std::uint32_t>(World_Block_FaceBit::ZN);
                    if (World_Block_IsTransparent(neighbour_blocks[static_cast<int>(World_Block_Face::ZP)])) blockface_bitmask |= static_cast<std::uint32_t>(World_Block_FaceBit::ZP);

                    if (blockface_bitmask == 0) continue;

                    // Chunk Mesh generation
                    World_GlobalXYZ block_offset = chunk_offset + World_GlobalXYZ(x, y, z);

                    for (int face = static_cast<int>(World_Block_Face::XN); face <= static_cast<int>(World_Block_Face::ZP); face++)
                    {
                        if (!(blockface_bitmask & (1u << face))) continue;

                        // Populate vertices 
                        const std::array<float, 12>& block_face = BLOCK_FACES[face];

                        glm::vec2 tile_map_offset = BLOCK_TILEMAP_OFFSETS[static_cast<int>(block.ID)][static_cast<int>(face)];

                        for (int i = 0; i < 4; i++)
                        {
                            int vertex_base = i * 3;

                            constexpr float w = 1.0f / 16.0f;

                            vertices.emplace_back(
                                ChunkMeshVertex(
                                    block_face[vertex_base + 0] + block_offset.x,
                                    block_face[vertex_base + 1] + block_offset.y,
                                    block_face[vertex_base + 2] + block_offset.z,
                                    tile_map_offset.x + ((i == 1 || i == 2) ? w : 0.0f),
                                    tile_map_offset.y + ((i == 2 || i == 3) ? w : 0.0f),
                                    static_cast<std::uint8_t>(face)
                                )
                            );
                        }

                        // Populate indices
                        std::uint32_t base_index = static_cast<std::uint32_t>(vertices.size() - 4);
                        indices.push_back(base_index + 0);
                        indices.push_back(base_index + 1);
                        indices.push_back(base_index + 2);
                        indices.push_back(base_index + 0);
                        indices.push_back(base_index + 2);
                        indices.push_back(base_index + 3);
                    }
                }
            }
        }
    }

    void LoadShaderProgram()
    {
        // Load shader sources
        auto vertex_shader_path = "./resource/shader/Chunk.vert.glsl";
        auto fragment_shader_path = "./resource/shader/Chunk.frag.glsl";

        auto vertex_shader_source_opt = IO_ReadFile(vertex_shader_path);
        if (vertex_shader_source_opt.has_value() == false)
        {
            std::println("Failed to read {}", vertex_shader_path);
            return;
        }
        const char* vertex_shader_source = vertex_shader_source_opt.value().c_str();

        auto fragment_shader_source_opt = IO_ReadFile(fragment_shader_path);
        if (fragment_shader_source_opt.has_value() == false)
        {
            std::println("Failed to read {}", fragment_shader_path);
            return;
        }
        const char* fragment_shader_source = fragment_shader_source_opt.value().c_str();

        // Create vertex shader
        GLint compile_status;
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
        glCompileShader(vertex_shader);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
        if (compile_status == GL_FALSE)
        {
            char info_log[512];
            glGetShaderInfoLog(vertex_shader, sizeof(info_log), nullptr, info_log);
            std::println("Error: Vertex Shader: {}", info_log);
            return;
        }

        // Create fragment shader
        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_status);
        if (compile_status == GL_FALSE)
        {
            char info_log[512];
            glGetShaderInfoLog(fragment_shader, sizeof(info_log), nullptr, info_log);
            std::println("Error: Fragment Shader: {}", info_log);
            return;
        }

        // Create shader program
        GLint link_status;
        GLuint program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &link_status);
        if (link_status == GL_FALSE)
        {
            char info_log[512];
            glGetProgramInfoLog(program, sizeof(info_log), nullptr, info_log);
            std::println("Error: Shader Program: {}", info_log);
            return;
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        i_ShaderProgram = program;
    }

    void LoadTexture()
    {
        auto image_opt = IO_ReadImage("./resource/texture/Blocks.png", true);

        if (image_opt.has_value() == false)
        {
            std::println("Failed to load ./resource/texture/Blocks.png");
            return;
        }

        auto& image = image_opt.value();

        GLint format = (image.ChannelNumbers == 4) ? GL_RGBA : GL_RGB;

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, format, image.Width, image.Height, 0, format, GL_UNSIGNED_BYTE, image.ImageData.data());
        //glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        i_Texture = texture;
    }

    void ChunkMesh::Create()
    {
        glGenVertexArrays(1, &VertexArrayID);
        glGenBuffers(1, &VertexBufferID);
        glGenBuffers(1, &IndexBufferID);

        glBindVertexArray(VertexArrayID);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferID);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkMeshVertex), reinterpret_cast<const void*>(offsetof(ChunkMeshVertex, X)));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ChunkMeshVertex), reinterpret_cast<const void*>(offsetof(ChunkMeshVertex, S)));
        glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(ChunkMeshVertex), reinterpret_cast<const void*>(offsetof(ChunkMeshVertex, F)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);

        IndicesCount = 0;
    }

    void ChunkMesh::Destroy()
    {
        glDeleteVertexArrays(1, &VertexArrayID);
        glDeleteBuffers(1, &VertexBufferID);
        glDeleteBuffers(1, &IndexBufferID);
        IndicesCount = 0;
    }
} // !namespace internal
