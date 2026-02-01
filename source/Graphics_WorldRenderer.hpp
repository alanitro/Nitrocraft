#pragma once

#include <unordered_map>
#include <glad/gl.h>
#include "World_Definitions.hpp"

class Camera;
struct Chunk;

class WorldRenderer
{
public:
    WorldRenderer() = default;
    ~WorldRenderer() = default;
    WorldRenderer(const WorldRenderer&) = delete;
    WorldRenderer& operator=(WorldRenderer&) = delete;

    void Initialize();
    void Terminate();

    void Render(const Camera& camera);

    void PushChunksToRender(ChunkID chunk_id, const Chunk* chunk);

private:
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

    GLuint m_ShaderProgram = 0;
    GLuint m_Texture       = 0;

    std::vector<ChunkMesh*>                 m_ChunksToRender;
    std::unordered_map<ChunkID, ChunkMesh>  m_ChunkMeshes;

    void GenerateMesh(std::vector<ChunkMeshVertex>& vertices, std::vector<std::uint32_t>& indices, WorldXYZ chunk_offset, const Chunk* chunk);
    void LoadShaderProgram();
    void LoadTexture();
};
