#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <atomic>
#include <glad/gl.h>
#include "World_chunk.hpp"

namespace nitrocraft::graphics
{

struct ChunkMeshVertexLayout
{
    float        x;  // Vertex position (x,y,z)
    float        y;
    float        z;
    float        s;  // Texture coordinate (s,t)
    float        t;
    std::uint8_t f;  // Face
    std::uint8_t l;  // Light
    std::uint8_t ao; // Ambient Occlusion Level [0,3]
};

struct ChunkCPUMesh
{
    world::Chunk*                      meshed_chunk;
    std::uint32_t                      completed_version;
    std::vector<ChunkMeshVertexLayout> vertices;
    std::vector<std::uint32_t>         indices;
};

ChunkCPUMesh GenerateChunkCPUMesh(const world::Chunk* chunk);

ChunkCPUMesh GenerateChunkCPUMesh_AmbientOcclusion(const world::Chunk* chunk);

struct ChunkGPUMeshHandle
{
    GLuint        vertex_array_id;
    GLuint        vertex_buffer_id;
    GLuint        index_buffer_id;
    std::uint32_t indices_count;

    ChunkGPUMeshHandle();
    ~ChunkGPUMeshHandle();
};

} // namespace nitrocraft::graphics