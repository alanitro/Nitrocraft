#pragma once

#include <cstdint>
#include <vector>
#include "World_Chunk.hpp"

struct World_Mesh_ChunkCPUMeshVertex
{
    float        X; // Vertex position (x,y,z)
    float        Y;
    float        Z;
    float        S; // Texture coordinate (s,t)
    float        T;
    std::uint8_t F; // Face
    std::uint8_t L; // Light
};

struct World_Mesh_ChunkCPUMesh
{
    std::vector<World_Mesh_ChunkCPUMeshVertex> Vertices;
    std::vector<std::uint32_t>                 Indices;
};

void World_Mesh_GenerateChunkCPUMesh(
    const World_Chunk* chunk,
    std::vector<World_Mesh_ChunkCPUMeshVertex>& vertices,
    std::vector<std::uint32_t>& indices
);
