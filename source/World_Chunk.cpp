#include "World_Chunk.hpp"

#include <algorithm>

World_Block World_Chunk::GetBlockAt(World_LocalXYZ local) const
{
    return Storage->Blocks.At(local.x, local.y, local.z);
}

World_Light World_Chunk::GetLightAt(World_LocalXYZ local) const
{
    return Storage->Lights.At(local.x, local.y, local.z);
}

World_Light World_Chunk::GetSunlightAt(World_LocalXYZ local) const
{
    return World_ExtractSunlight(Storage->Lights.At(local.x, local.y, local.z));
}

World_Light World_Chunk::GetPointlightAt(World_LocalXYZ local) const
{
    return World_ExtractPointlight(Storage->Lights.At(local.x, local.y, local.z));
}

void World_Chunk::SetBlockAt(World_LocalXYZ local, World_Block block)
{
    Storage->Blocks.At(local.x, local.y, local.z) = block;
}

void World_Chunk::SetLightAt(World_LocalXYZ local, World_Light sunlight, World_Light pointlight)
{
    Storage->Lights.At(local.x, local.y, local.z) = ((sunlight << 0) & 0x0F) | ((pointlight << 4) & 0xF0);
}

void World_Chunk::SetSunlightAt(World_LocalXYZ local, World_Light sunlight)
{
    auto& light = Storage->Lights.At(local.x, local.y, local.z);

    light = (light & 0xF0) | ((sunlight << 0) & 0x0F);
}

void World_Chunk::SetPointlightAt(World_LocalXYZ local, World_Light pointlight)
{
    auto& light = Storage->Lights.At(local.x, local.y, local.z);

    light = (light & 0x0F) | ((pointlight << 4) & 0xF0);
}

int World_Chunk::GetHeightAt(int local_x, int local_z) const
{
    return Storage->Heights.At(local_x, local_z);
}

int World_Chunk::GetMaxHeight() const
{
    return *std::max_element(Storage->Heights.begin(), Storage->Heights.end());;
}

std::array<World_Block, static_cast<std::size_t>(World_Block_CrossNeighbour::Count)>
World_Chunk::GetCrossNeighbourBlocksAt(World_LocalXYZ local) const
{
    int& x = local.x;
    int& y = local.y;
    int& z = local.z;

    World_Chunk* cxn = Neighbours[(std::size_t)World_Chunk_Neighbour::XNZ0];
    World_Chunk* cxp = Neighbours[(std::size_t)World_Chunk_Neighbour::XPZ0];
    World_Chunk* czn = Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZN];
    World_Chunk* czp = Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZP];

    return std::array<World_Block, static_cast<std::size_t>(World_Block_CrossNeighbour::Count)>
    {
        (x != 0)                        ? GetBlockAt(World_LocalXYZ(x - 1, y, z)) : cxn->GetBlockAt(World_LocalXYZ(World_CHUNK_X_SIZE - 1, y, z)),
        (x != World_CHUNK_X_SIZE - 1)   ? GetBlockAt(World_LocalXYZ(x + 1, y, z)) : cxp->GetBlockAt(World_LocalXYZ(0, y, z)),
        (y != 0)                        ? GetBlockAt(World_LocalXYZ(x, y - 1, z)) : World_Block(World_Block_ID::AIR),
        (y != World_CHUNK_Y_SIZE - 1)   ? GetBlockAt(World_LocalXYZ(x, y + 1, z)) : World_Block(World_Block_ID::AIR),
        (z != 0)                        ? GetBlockAt(World_LocalXYZ(x, y, z - 1)) : czn->GetBlockAt(World_LocalXYZ(x, y, World_CHUNK_Z_SIZE - 1)),
        (z != World_CHUNK_Z_SIZE - 1)   ? GetBlockAt(World_LocalXYZ(x, y, z + 1)) : czp->GetBlockAt(World_LocalXYZ(x, y, 0))
    };
}

std::array<World_Light, static_cast<std::size_t>(World_Block_CrossNeighbour::Count)>
World_Chunk::GetCrossNeighbourLightsAt(World_LocalXYZ local) const
{
    int& x = local.x;
    int& y = local.y;
    int& z = local.z;

    World_Chunk* cxn = Neighbours[(std::size_t)World_Chunk_Neighbour::XNZ0];
    World_Chunk* cxp = Neighbours[(std::size_t)World_Chunk_Neighbour::XPZ0];
    World_Chunk* czn = Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZN];
    World_Chunk* czp = Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZP];

    return std::array<World_Light, static_cast<std::size_t>(World_Block_CrossNeighbour::Count)>
    {
        (x != 0)                        ? GetLightAt(World_LocalXYZ(x - 1, y, z)) : cxn->GetLightAt(World_LocalXYZ(World_CHUNK_X_SIZE - 1, y, z)),
        (x != World_CHUNK_X_SIZE - 1)   ? GetLightAt(World_LocalXYZ(x + 1, y, z)) : cxp->GetLightAt(World_LocalXYZ(0, y, z)),
        (y != 0)                        ? GetLightAt(World_LocalXYZ(x, y - 1, z)) : World_LIGHT_LEVEL_MIN,
        (y != World_CHUNK_Y_SIZE - 1)   ? GetLightAt(World_LocalXYZ(x, y + 1, z)) : World_LIGHT_LEVEL_SUN,
        (z != 0)                        ? GetLightAt(World_LocalXYZ(x, y, z - 1)) : czn->GetLightAt(World_LocalXYZ(x, y, World_CHUNK_Z_SIZE - 1)),
        (z != World_CHUNK_Z_SIZE - 1)   ? GetLightAt(World_LocalXYZ(x, y, z + 1)) : czp->GetLightAt(World_LocalXYZ(x, y, 0))
    };
}

std::array<World_Block, static_cast<std::size_t>(World_Block_WholeNeighbour::Count)>
World_Chunk::GetWholeNeighbourBlocksAt(World_LocalXYZ local) const
{
    const int x = local.x;
    const int y = local.y;
    const int z = local.z;

    const World_Chunk* cxn   = Neighbours[(std::size_t)World_Chunk_Neighbour::XNZ0];
    const World_Chunk* cxp   = Neighbours[(std::size_t)World_Chunk_Neighbour::XPZ0];
    const World_Chunk* czn   = Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZN];
    const World_Chunk* czp   = Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZP];
    const World_Chunk* cxnzn = Neighbours[(std::size_t)World_Chunk_Neighbour::XNZN];
    const World_Chunk* cxpzn = Neighbours[(std::size_t)World_Chunk_Neighbour::XPZN];
    const World_Chunk* cxnzp = Neighbours[(std::size_t)World_Chunk_Neighbour::XNZP];
    const World_Chunk* cxpzp = Neighbours[(std::size_t)World_Chunk_Neighbour::XPZP];

    constexpr auto air = World_Block(World_Block_ID::AIR);

    // Fetch a block with possible X/Z chunk crossing. (No vertical chunking: Y outside => AIR)
    auto B = [&](int nx, int ny, int nz) -> World_Block
    {
        if (ny < 0 || ny >= World_CHUNK_Y_SIZE) return air;

        const bool x_neg = (nx < 0);
        const bool x_pos = (nx >= World_CHUNK_X_SIZE);
        const bool z_neg = (nz < 0);
        const bool z_pos = (nz >= World_CHUNK_Z_SIZE);

        if (!x_neg && !x_pos && !z_neg && !z_pos) return GetBlockAt(World_LocalXYZ(nx, ny, nz));

        const World_Chunk* c = this;
        int lx = nx;
        int lz = nz;

        // Choose chunk + local coords
        if      (x_neg && z_neg)  { c = cxnzn; lx = World_CHUNK_X_SIZE - 1; lz = World_CHUNK_Z_SIZE - 1; }
        else if (x_pos && z_neg)  { c = cxpzn; lx = 0;                      lz = World_CHUNK_Z_SIZE - 1; }
        else if (x_neg && z_pos)  { c = cxnzp; lx = World_CHUNK_X_SIZE - 1; lz = 0; }
        else if (x_pos && z_pos)  { c = cxpzp; lx = 0;                      lz = 0; }
        else if (x_neg)           { c = cxn;   lx = World_CHUNK_X_SIZE - 1; }
        else if (x_pos)           { c = cxp;   lx = 0; }
        else if (z_neg)           { c = czn;   lz = World_CHUNK_Z_SIZE - 1; }
        else                      { c = czp;   lz = 0; }

        if (c == nullptr) return air;

        return c->GetBlockAt(World_LocalXYZ(lx, ny, lz));
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
