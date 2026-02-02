#include "World.hpp"

#include "Graphics_Camera.hpp"

void World::Initialize()
{
    TerrainGenerator::Initialize(12345);

    Update_ActiveArea(ChunkID{ 0,0,0 });
}

void World::Terminate()
{
    m_ChunkMap.clear();
}

void World::Update(Camera& camera)
{
    Update_ActiveArea(FromWorldPositionToChunkID(camera.GetPosition()));
}

Block World::GetBlockAt(WorldPosition position) const
{
    auto chunk = GetChunkAt(position);

    if (chunk == nullptr) return Block{ BlockID::AIR };

    return chunk->GetBlockAt(FromWorldPositionToChunkPosition(position));
}

Chunk* World::GetChunkAt(WorldPosition position) const
{
    auto iter = m_ChunkMap.find(FromWorldPositionToChunkID(position));

    if (iter == m_ChunkMap.end()) return nullptr;

    return iter->second.get();
}

void World::Update_ActiveArea(ChunkID center_chunk_id)
{
    const ChunkID chunk_offset_id = center_chunk_id - ChunkID(WORLD_LOADING_RADIUS, 0, WORLD_LOADING_RADIUS);

    for (int iz = 0; iz < WORLD_LOADING_DIAMETER; iz++)
    {
        for (int ix = 0; ix < WORLD_LOADING_DIAMETER; ix++)
        {
            auto chunk_id = ChunkID(chunk_offset_id + ChunkID(ix, 0, iz));

            auto iter = m_ChunkMap.find(chunk_id);

            if (iter != m_ChunkMap.end())
            {
                m_ActiveArea.At(ix, iz) = iter->second.get();

                continue;
            }
            else
            {
                m_ChunkMap.erase(chunk_id);
            }

            auto [it, inserted] = m_ChunkMap.emplace(chunk_id, std::make_unique<Chunk>(chunk_id));

            Chunk* new_chunk = it->second.get();

            TerrainGenerator::GenerateTerrain(new_chunk);

            m_ActiveArea.At(ix, iz) = new_chunk;
        }
    }

    for (int iz = 1; iz < WORLD_LOADING_DIAMETER - 1; iz++)
    {
        for (int ix = 1; ix < WORLD_LOADING_DIAMETER - 1; ix++)
        {
            Chunk* chunk = m_ActiveArea.At(ix, iz);
            chunk->NeighbourXNZ0 = m_ActiveArea.At(ix - 1, iz    );
            chunk->NeighbourXPZ0 = m_ActiveArea.At(ix + 1, iz    );
            chunk->NeighbourX0ZN = m_ActiveArea.At(ix    , iz - 1);
            chunk->NeighbourX0ZP = m_ActiveArea.At(ix    , iz + 1);
            chunk->NeighbourXNZN = m_ActiveArea.At(ix - 1, iz - 1);
            chunk->NeighbourXPZN = m_ActiveArea.At(ix + 1, iz - 1);
            chunk->NeighbourXNZP = m_ActiveArea.At(ix - 1, iz + 1);
            chunk->NeighbourXPZP = m_ActiveArea.At(ix + 1, iz + 1);
            chunk->NeighboursSet = true;
        }
    }
}
