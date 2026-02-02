#pragma once

#include <memory>
#include <unordered_map>
#include "World_Definitions.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"
#include "World_TerrainGenerator.hpp"
#include "Utility_Array2D.hpp"

class Camera;

class World
{
public:
    World() = default;
    ~World() = default;
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    void Initialize();
    void Terminate();
    void Update(Camera& camera);

    Block GetBlockAt(WorldPosition position) const;

    Chunk* GetChunkAt(WorldPosition position) const;

    const auto& GetActiveArea() const
    {
        return m_ActiveArea;
    }

private:
    std::unordered_map<ChunkID, std::unique_ptr<Chunk>> m_ChunkMap;

    Array2D<Chunk*, WORLD_LOADING_DIAMETER, WORLD_LOADING_DIAMETER> m_ActiveArea{};

    void Update_ActiveArea(ChunkID center_chunk_id);
};
