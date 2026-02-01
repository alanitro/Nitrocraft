#include "World_Chunk.hpp"

Block Chunk::GetBlockAt(int x, int y, int z) const
{
    return Blocks.At(x, y, z);
}

Block Chunk::GetBlockAt(ChunkXYZ position) const
{
    return Blocks.At(position.x, position.y, position.z);
}

std::array<Block, (int)BlockNeighbour::COUNT> Chunk::GetNeighbourBlocksAt(ChunkXYZ position) const
{
    int& x = position.x;
    int& y = position.y;
    int& z = position.z;

    Chunk* cxn = NeighbourXNZ0;
    Chunk* cxp = NeighbourXPZ0;
    Chunk* czn = NeighbourX0ZN;
    Chunk* czp = NeighbourX0ZP;

    return std::array<Block, (int)BlockNeighbour::COUNT>
    {
        (x != 0)                        ? GetBlockAt(x - 1, y, z) : cxn->GetBlockAt(WORLD_CHUNK_X_SIZE - 1, y, z),
        (x != WORLD_CHUNK_X_SIZE - 1)   ? GetBlockAt(x + 1, y, z) : cxp->GetBlockAt(0, y, z),
        (y != 0)                        ? GetBlockAt(x, y - 1, z) : BlockID::AIR,
        (y != WORLD_CHUNK_Y_SIZE - 1)   ? GetBlockAt(x, y + 1, z) : BlockID::AIR,
        (z != 0)                        ? GetBlockAt(x, y, z - 1) : czn->GetBlockAt(x, y, WORLD_CHUNK_Z_SIZE - 1),
        (z != WORLD_CHUNK_Z_SIZE - 1)   ? GetBlockAt(x, y, z + 1) : czp->GetBlockAt(x, y, 0)
    };
}
