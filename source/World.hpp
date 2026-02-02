#pragma once

#include <memory>
#include <unordered_map>
#include "World_Definitions.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"
#include "World_TerrainGenerator.hpp"

class World
{
public:
    World() = default;
    ~World() = default;
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    void Initialize();
    void Terminate();

    Block GetBlockAt(WorldXYZ position) const;

    Chunk* GetChunkAt(WorldXYZ position) const;

private:
    std::unordered_map<ChunkID, std::unique_ptr<Chunk>> m_ChunkMap;
};
