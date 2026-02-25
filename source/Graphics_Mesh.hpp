#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <atomic>
#include <glad/gl.h>

struct World_Chunk;

struct Graphics_ChunkMeshVertexLayout
{
    float        X;  // Vertex position (x,y,z)
    float        Y;
    float        Z;
    float        S;  // Texture coordinate (s,t)
    float        T;
    std::uint8_t F;  // Face
    std::uint8_t L;  // Light
    std::uint8_t AO; // Ambient Occlusion Level [0,3]
};

struct Graphics_ChunkCPUMesh
{
    World_Chunk*  MeshedChunk;
    std::uint32_t RequestedVersion;
    std::vector<Graphics_ChunkMeshVertexLayout> Vertices;
    std::vector<std::uint32_t>                  Indices;
};

Graphics_ChunkCPUMesh Graphics_Mesh_GenerateChunkCPUMesh(const World_Chunk* chunk);

Graphics_ChunkCPUMesh Graphics_Mesh_GenerateChunkCPUMesh_AmbientOcclusion(const World_Chunk* chunk);

struct Graphics_ChunkGPUMeshHandle
{
    GLuint        VertexArrayID;
    GLuint        VertexBufferID;
    GLuint        IndexBufferID;
    std::uint32_t IndicesCount;

    Graphics_ChunkGPUMeshHandle();

    ~Graphics_ChunkGPUMeshHandle();
};
