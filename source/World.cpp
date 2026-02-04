#include "World.hpp"

#include "Graphics_Camera.hpp"

namespace
{
    std::unordered_map<ChunkID, std::unique_ptr<Chunk>>             i_ChunkMap;
    Array2D<Chunk*, WORLD_LOADING_DIAMETER, WORLD_LOADING_DIAMETER> i_ActiveArea{};

    void Update_ActiveArea(ChunkID center_chunk_id);
}

void World::Initialize()
{
    TerrainGenerator_Initialize(12345);

    Update_ActiveArea(ChunkID{ 0,0,0 });
}

void World::Terminate()
{
    i_ChunkMap.clear();
}

void World::Update(Camera& camera)
{
    Update_ActiveArea(FromWorldPositionToChunkID(camera.GetPosition()));
}

Block World::GetBlockAt(WorldPosition position)
{
    auto chunk = GetChunkAt(position);

    if (chunk == nullptr) return Block{ BlockID::AIR };

    return chunk->GetBlockAt(FromWorldPositionToChunkPosition(position));
}

Chunk* World::GetChunkAt(WorldPosition position)
{
    auto iter = i_ChunkMap.find(FromWorldPositionToChunkID(position));

    if (iter == i_ChunkMap.end()) return nullptr;

    return iter->second.get();
}

const Array2D<Chunk*, WORLD_LOADING_DIAMETER, WORLD_LOADING_DIAMETER>& World::GetActiveArea()
{
    return i_ActiveArea;
}

namespace
{
void Update_ActiveArea(ChunkID center_chunk_id)
{
    const ChunkID chunk_offset_id = center_chunk_id - ChunkID(WORLD_LOADING_RADIUS, 0, WORLD_LOADING_RADIUS);

    for (int iz = 0; iz < WORLD_LOADING_DIAMETER; iz++)
    {
        for (int ix = 0; ix < WORLD_LOADING_DIAMETER; ix++)
        {
            auto chunk_id = ChunkID(chunk_offset_id + ChunkID(ix, 0, iz));

            auto iter = i_ChunkMap.find(chunk_id);

            if (iter != i_ChunkMap.end())
            {
                i_ActiveArea.At(ix, iz) = iter->second.get();

                continue;
            }
            else
            {
                i_ChunkMap.erase(chunk_id);
            }

            auto [it, inserted] = i_ChunkMap.emplace(chunk_id, std::make_unique<Chunk>(chunk_id));

            Chunk* new_chunk = it->second.get();

            TerrainGenerator_GenerateTerrain(new_chunk);

            i_ActiveArea.At(ix, iz) = new_chunk;
        }
    }

    for (int iz = 1; iz < WORLD_LOADING_DIAMETER - 1; iz++)
    {
        for (int ix = 1; ix < WORLD_LOADING_DIAMETER - 1; ix++)
        {
            Chunk* chunk = i_ActiveArea.At(ix, iz);
            chunk->NeighbourXNZ0 = i_ActiveArea.At(ix - 1, iz);
            chunk->NeighbourXPZ0 = i_ActiveArea.At(ix + 1, iz);
            chunk->NeighbourX0ZN = i_ActiveArea.At(ix, iz - 1);
            chunk->NeighbourX0ZP = i_ActiveArea.At(ix, iz + 1);
            chunk->NeighbourXNZN = i_ActiveArea.At(ix - 1, iz - 1);
            chunk->NeighbourXPZN = i_ActiveArea.At(ix + 1, iz - 1);
            chunk->NeighbourXNZP = i_ActiveArea.At(ix - 1, iz + 1);
            chunk->NeighbourXPZP = i_ActiveArea.At(ix + 1, iz + 1);
            chunk->NeighboursSet = true;
        }
    }
}
}