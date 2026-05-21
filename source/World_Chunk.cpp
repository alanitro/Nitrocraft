#include "World_Chunk.hpp"

#include <algorithm>

namespace nitrocraft::world
{

Block Chunk::GetBlockAt(LocalXYZ local) const
{
    return storage->blocks.At(local.x, local.y, local.z);
}

LightLevel Chunk::GetLightAt(LocalXYZ local) const
{
    return storage->lights.At(local.x, local.y, local.z);
}

LightLevel Chunk::GetSunlightAt(LocalXYZ local) const
{
    return ExtractSunlight(storage->lights.At(local.x, local.y, local.z));
}

LightLevel Chunk::GetPointlightAt(LocalXYZ local) const
{
    return ExtractPointlight(storage->lights.At(local.x, local.y, local.z));
}

void Chunk::SetBlockAt(LocalXYZ local, Block block)
{
    storage->blocks.At(local.x, local.y, local.z) = block;
}

void Chunk::SetLightAt(LocalXYZ local, LightLevel sunlight, LightLevel pointlight)
{
    storage->lights.At(local.x, local.y, local.z) = ((sunlight << 0) & 0x0F) | ((pointlight << 4) & 0xF0);
}

void Chunk::SetSunlightAt(LocalXYZ local, LightLevel sunlight)
{
    auto& light = storage->lights.At(local.x, local.y, local.z);

    light = (light & 0xF0) | ((sunlight << 0) & 0x0F);
}

void Chunk::SetPointlightAt(LocalXYZ local, LightLevel pointlight)
{
    auto& light = storage->lights.At(local.x, local.y, local.z);

    light = (light & 0x0F) | ((pointlight << 4) & 0xF0);
}

int Chunk::GetHeightAt(int local_x, int local_z) const
{
    return storage->heights.At(local_x, local_z);
}

int Chunk::GetMaxHeight() const
{
    return *std::max_element(storage->heights.begin(), storage->heights.end());;
}

std::array<Block, static_cast<std::size_t>(BlockCrossNeighbour::Count)>
Chunk::GetCrossNeighbourBlocksAt(LocalXYZ local) const
{
    int& x = local.x;
    int& y = local.y;
    int& z = local.z;

    Chunk* cxn = neighbours[(std::size_t)ChunkNeighbour::XNZ0];
    Chunk* cxp = neighbours[(std::size_t)ChunkNeighbour::XPZ0];
    Chunk* czn = neighbours[(std::size_t)ChunkNeighbour::X0ZN];
    Chunk* czp = neighbours[(std::size_t)ChunkNeighbour::X0ZP];

    return std::array<Block, static_cast<std::size_t>(BlockCrossNeighbour::Count)>
    {
        (x != 0)                ? GetBlockAt(LocalXYZ(x - 1, y, z)) : cxn->GetBlockAt(LocalXYZ(CHUNK_X_SIZE - 1, y, z)),
        (x != CHUNK_X_SIZE - 1) ? GetBlockAt(LocalXYZ(x + 1, y, z)) : cxp->GetBlockAt(LocalXYZ(0, y, z)),
        (y != 0)                ? GetBlockAt(LocalXYZ(x, y - 1, z)) : Block(BlockID::AIR),
        (y != CHUNK_Y_SIZE - 1) ? GetBlockAt(LocalXYZ(x, y + 1, z)) : Block(BlockID::AIR),
        (z != 0)                ? GetBlockAt(LocalXYZ(x, y, z - 1)) : czn->GetBlockAt(LocalXYZ(x, y, CHUNK_Z_SIZE - 1)),
        (z != CHUNK_Z_SIZE - 1) ? GetBlockAt(LocalXYZ(x, y, z + 1)) : czp->GetBlockAt(LocalXYZ(x, y, 0))
    };
}

std::array<LightLevel, static_cast<std::size_t>(BlockCrossNeighbour::Count)>
Chunk::GetCrossNeighbourLightsAt(LocalXYZ local) const
{
    int& x = local.x;
    int& y = local.y;
    int& z = local.z;

    Chunk* cxn = neighbours[(std::size_t)ChunkNeighbour::XNZ0];
    Chunk* cxp = neighbours[(std::size_t)ChunkNeighbour::XPZ0];
    Chunk* czn = neighbours[(std::size_t)ChunkNeighbour::X0ZN];
    Chunk* czp = neighbours[(std::size_t)ChunkNeighbour::X0ZP];

    return std::array<LightLevel, static_cast<std::size_t>(BlockCrossNeighbour::Count)>
    {
        (x != 0)                ? GetLightAt(LocalXYZ(x - 1, y, z)) : cxn->GetLightAt(LocalXYZ(CHUNK_X_SIZE - 1, y, z)),
        (x != CHUNK_X_SIZE - 1) ? GetLightAt(LocalXYZ(x + 1, y, z)) : cxp->GetLightAt(LocalXYZ(0, y, z)),
        (y != 0)                ? GetLightAt(LocalXYZ(x, y - 1, z)) : LIGHT_LEVEL_MIN,
        (y != CHUNK_Y_SIZE - 1) ? GetLightAt(LocalXYZ(x, y + 1, z)) : LIGHT_LEVEL_SUN,
        (z != 0)                ? GetLightAt(LocalXYZ(x, y, z - 1)) : czn->GetLightAt(LocalXYZ(x, y, CHUNK_Z_SIZE - 1)),
        (z != CHUNK_Z_SIZE - 1) ? GetLightAt(LocalXYZ(x, y, z + 1)) : czp->GetLightAt(LocalXYZ(x, y, 0))
    };
}

std::array<Block, static_cast<std::size_t>(BlockWholeNeighbour::Count)>
Chunk::GetWholeNeighbourBlocksAt(LocalXYZ local) const
{
    const int x = local.x;
    const int y = local.y;
    const int z = local.z;

    const Chunk* cxn   = neighbours[(std::size_t)ChunkNeighbour::XNZ0];
    const Chunk* cxp   = neighbours[(std::size_t)ChunkNeighbour::XPZ0];
    const Chunk* czn   = neighbours[(std::size_t)ChunkNeighbour::X0ZN];
    const Chunk* czp   = neighbours[(std::size_t)ChunkNeighbour::X0ZP];
    const Chunk* cxnzn = neighbours[(std::size_t)ChunkNeighbour::XNZN];
    const Chunk* cxpzn = neighbours[(std::size_t)ChunkNeighbour::XPZN];
    const Chunk* cxnzp = neighbours[(std::size_t)ChunkNeighbour::XNZP];
    const Chunk* cxpzp = neighbours[(std::size_t)ChunkNeighbour::XPZP];

    constexpr auto air = Block(BlockID::AIR);

    // Fetch a block with possible X/Z chunk crossing. (No vertical chunking: Y outside => AIR)
    auto B = [&](int nx, int ny, int nz) -> Block
    {
        if (ny < 0 || ny >= CHUNK_Y_SIZE) return air;

        const bool x_neg = (nx < 0);
        const bool x_pos = (nx >= CHUNK_X_SIZE);
        const bool z_neg = (nz < 0);
        const bool z_pos = (nz >= CHUNK_Z_SIZE);

        if (!x_neg && !x_pos && !z_neg && !z_pos) return GetBlockAt(LocalXYZ(nx, ny, nz));

        const Chunk* c = this;
        int lx = nx;
        int lz = nz;

        // Choose chunk + local coords
        if      (x_neg && z_neg)  { c = cxnzn; lx = CHUNK_X_SIZE - 1; lz = CHUNK_Z_SIZE - 1; }
        else if (x_pos && z_neg)  { c = cxpzn; lx = 0;                      lz = CHUNK_Z_SIZE - 1; }
        else if (x_neg && z_pos)  { c = cxnzp; lx = CHUNK_X_SIZE - 1; lz = 0; }
        else if (x_pos && z_pos)  { c = cxpzp; lx = 0;                      lz = 0; }
        else if (x_neg)           { c = cxn;   lx = CHUNK_X_SIZE - 1; }
        else if (x_pos)           { c = cxp;   lx = 0; }
        else if (z_neg)           { c = czn;   lz = CHUNK_Z_SIZE - 1; }
        else                      { c = czp;   lz = 0; }

        if (c == nullptr) return air;

        return c->GetBlockAt(LocalXYZ(lx, ny, lz));
    };

    return
    {
        B(x - 1, y,     z),     // XnYoZo
        B(x + 1, y,     z),     // XpYoZo
        B(x,     y - 1, z),     // XoYnZo
        B(x,     y + 1, z),     // XoYpZo
        B(x,     y,     z - 1), // XoYoZn
        B(x,     y,     z + 1), // XoYoZp

        B(x - 1, y,     z - 1), // XnYoZn
        B(x + 1, y,     z - 1), // XpYoZn
        B(x - 1, y,     z + 1), // XnYoZp
        B(x + 1, y,     z + 1), // XpYoZp

        B(x,     y - 1, z - 1), // XoYnZn
        B(x,     y + 1, z - 1), // XoYpZn
        B(x,     y - 1, z + 1), // XoYnZp
        B(x,     y + 1, z + 1), // XoYpZp

        B(x - 1, y - 1, z),     // XnYnZo
        B(x + 1, y - 1, z),     // XpYnZo
        B(x - 1, y + 1, z),     // XnYpZo
        B(x + 1, y + 1, z),     // XpYpZo

        B(x - 1, y - 1, z - 1), // XnYnZn
        B(x + 1, y - 1, z - 1), // XpYnZn
        B(x - 1, y + 1, z - 1), // XnYpZn
        B(x + 1, y + 1, z - 1), // XpYpZn
        B(x - 1, y - 1, z + 1), // XnYnZp
        B(x + 1, y - 1, z + 1), // XpYnZp
        B(x - 1, y + 1, z + 1), // XnYpZp
        B(x + 1, y + 1, z + 1), // XpYpZp
    };
}

} // namespace nitrocraft::world
