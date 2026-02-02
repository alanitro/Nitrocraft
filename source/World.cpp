#include "World.hpp"

void World::Initialize()
{
    TerrainGenerator::Initialize(12345);

    auto chunk = std::make_unique<Chunk>();

    Chunk* chunk_xnz0 = new Chunk;
    Chunk* chunk_xpz0 = new Chunk;
    Chunk* chunk_x0zn = new Chunk;
    Chunk* chunk_x0zp = new Chunk;
    Chunk* chunk_xnzn = new Chunk;
    Chunk* chunk_xpzn = new Chunk;
    Chunk* chunk_xnzp = new Chunk;
    Chunk* chunk_xpzp = new Chunk;
    chunk_xnz0->Blocks.Fill(BlockID::AIR);
    chunk_xpz0->Blocks.Fill(BlockID::AIR);
    chunk_x0zn->Blocks.Fill(BlockID::AIR);
    chunk_x0zp->Blocks.Fill(BlockID::AIR);
    chunk_xnzn->Blocks.Fill(BlockID::AIR);
    chunk_xpzn->Blocks.Fill(BlockID::AIR);
    chunk_xnzp->Blocks.Fill(BlockID::AIR);
    chunk_xpzp->Blocks.Fill(BlockID::AIR);

    chunk->NeighbourXNZ0 = chunk_xnz0;
    chunk->NeighbourXPZ0 = chunk_xpz0;
    chunk->NeighbourX0ZN = chunk_x0zn;
    chunk->NeighbourX0ZP = chunk_x0zp;
    chunk->NeighbourXNZN = chunk_xnzn;
    chunk->NeighbourXPZN = chunk_xpzn;
    chunk->NeighbourXNZP = chunk_xnzp;
    chunk->NeighbourXPZP = chunk_xpzp;

    TerrainGenerator::GenerateTerrain(WorldXYZ(0,0,0), chunk.get());

    m_ChunkMap.emplace(ChunkID(0, 0, 0), std::move(chunk));
}

void World::Terminate()
{
    m_ChunkMap.clear();
}

Block World::GetBlockAt(WorldXYZ position) const
{
    auto chunk = GetChunkAt(position);

    if (chunk == nullptr) return Block{ BlockID::AIR };

    return chunk->GetBlockAt(ToChunkXYZ(position));
}

Chunk* World::GetChunkAt(WorldXYZ position) const
{
    auto iter = m_ChunkMap.find(ToChunkID(position));

    if (iter == m_ChunkMap.end()) return nullptr;

    return iter->second.get();
}
