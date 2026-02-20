#pragma once

#include <cstdint>
#include <vector>
#include <atomic>
#include <glad/gl.h>

struct World_Chunk;

struct Graphics_ChunkCPUMeshVertexLayout
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
    std::vector<Graphics_ChunkCPUMeshVertexLayout>  Vertices;
    std::vector<std::uint32_t>                      Indices;
};

void Graphics_Mesh_GenerateChunkCPUMesh(const World_Chunk* chunk, Graphics_ChunkCPUMesh& mesh);

void Graphics_Mesh_GenerateChunkCPUMesh_AmbientOcclusion(const World_Chunk* chunk, Graphics_ChunkCPUMesh& mesh);

enum Graphics_ChunkGPUMeshingStage
{
    Empty,

    MeshingInProgress,
    MeshingComplete,

    UploadingInProgress,
    UploadingComplete,
};

struct Graphics_ChunkGPUMeshHandle
{
    std::uint32_t Version;
    std::atomic<Graphics_ChunkGPUMeshingStage> Stage;

    GLuint        VertexArrayID;
    GLuint        VertexBufferID;
    GLuint        IndexBufferID;
    std::uint32_t IndicesCount;

    explicit Graphics_ChunkGPUMeshHandle(std::uint32_t storage_version = 0);

    ~Graphics_ChunkGPUMeshHandle();

    void UploadCPUMeshToGPU(const Graphics_ChunkCPUMesh& cpumesh);
};
