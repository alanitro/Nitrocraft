#include "World_Chunk.hpp"

World_Block World_Chunk::GetBlockAt(World_LocalXYZ local) const
{
    return Payload->Blocks.At(local.x, local.y, local.z);
}

World_Light World_Chunk::GetLightAt(World_LocalXYZ local) const
{
    return Payload->Lights.At(local.x, local.y, local.z);
}

World_Light World_Chunk::GetSunlightAt(World_LocalXYZ local) const
{
    return World_ExtractSunlight(Payload->Lights.At(local.x, local.y, local.z));
}

World_Light World_Chunk::GetPointlightAt(World_LocalXYZ local) const
{
    return World_ExtractPointlight(Payload->Lights.At(local.x, local.y, local.z));
}

void World_Chunk::SetBlockAt(World_LocalXYZ local, World_Block block)
{
    Payload->Blocks.At(local.x, local.y, local.z) = block;
}

void World_Chunk::SetLightAt(World_LocalXYZ local, World_Light sunlight, World_Light pointlight)
{
    Payload->Lights.At(local.x, local.y, local.z) = ((sunlight << 0) & 0x0F) | ((pointlight << 4) & 0xF0);
}

void World_Chunk::SetSunlightAt(World_LocalXYZ local, World_Light sunlight)
{
    auto& light = Payload->Lights.At(local.x, local.y, local.z);

    light = (light & 0xF0) | ((sunlight << 0) & 0x0F);
}

void World_Chunk::SetPointlightAt(World_LocalXYZ local, World_Light pointlight)
{
    auto& light = Payload->Lights.At(local.x, local.y, local.z);

    light = (light & 0x0F) | ((pointlight << 4) & 0xF0);
}

int World_Chunk::GetHeightAt(int local_x, int local_z)
{
    return Payload->Heights.At(local_x, local_z);
}

void World_Chunk::SetHeightAt(int local_x, int local_z, std::uint8_t height)
{
    Payload->Heights.At(local_x, local_z) = static_cast<std::uint8_t>(height);
}

std::array<World_Block, static_cast<std::size_t>(World_Block_Neighbour::COUNT)> World_Chunk::GetNeighbourBlocksAt(World_LocalXYZ local) const
{
    int& x = local.x;
    int& y = local.y;
    int& z = local.z;

    World_Chunk* cxn = Neighbours[(std::size_t)World_Chunk_Neighbour::XNZ0];
    World_Chunk* cxp = Neighbours[(std::size_t)World_Chunk_Neighbour::XPZ0];
    World_Chunk* czn = Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZN];
    World_Chunk* czp = Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZP];

    return std::array<World_Block, static_cast<std::size_t>(World_Block_Neighbour::COUNT)>
    {
        (x != 0)                        ? GetBlockAt(World_LocalXYZ(x - 1, y, z)) : cxn->GetBlockAt(World_LocalXYZ(World_CHUNK_X_SIZE - 1, y, z)),
        (x != World_CHUNK_X_SIZE - 1)   ? GetBlockAt(World_LocalXYZ(x + 1, y, z)) : cxp->GetBlockAt(World_LocalXYZ(0, y, z)),
        (y != 0)                        ? GetBlockAt(World_LocalXYZ(x, y - 1, z)) : World_Block(World_Block_ID::AIR),
        (y != World_CHUNK_Y_SIZE - 1)   ? GetBlockAt(World_LocalXYZ(x, y + 1, z)) : World_Block(World_Block_ID::AIR),
        (z != 0)                        ? GetBlockAt(World_LocalXYZ(x, y, z - 1)) : czn->GetBlockAt(World_LocalXYZ(x, y, World_CHUNK_Z_SIZE - 1)),
        (z != World_CHUNK_Z_SIZE - 1)   ? GetBlockAt(World_LocalXYZ(x, y, z + 1)) : czp->GetBlockAt(World_LocalXYZ(x, y, 0))
    };
}

std::array<World_Light, static_cast<std::size_t>(World_Block_Neighbour::COUNT)> World_Chunk::GetNeighbourLightsAt(World_LocalXYZ local) const
{
    int& x = local.x;
    int& y = local.y;
    int& z = local.z;

    World_Chunk* cxn = Neighbours[(std::size_t)World_Chunk_Neighbour::XNZ0];
    World_Chunk* cxp = Neighbours[(std::size_t)World_Chunk_Neighbour::XPZ0];
    World_Chunk* czn = Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZN];
    World_Chunk* czp = Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZP];

    return std::array<World_Light, static_cast<std::size_t>(World_Block_Neighbour::COUNT)>
    {
        (x != 0)                        ? GetLightAt(World_LocalXYZ(x - 1, y, z)) : cxn->GetLightAt(World_LocalXYZ(World_CHUNK_X_SIZE - 1, y, z)),
        (x != World_CHUNK_X_SIZE - 1)   ? GetLightAt(World_LocalXYZ(x + 1, y, z)) : cxp->GetLightAt(World_LocalXYZ(0, y, z)),
        (y != 0)                        ? GetLightAt(World_LocalXYZ(x, y - 1, z)) : World_LIGHT_LEVEL_MIN,
        (y != World_CHUNK_Y_SIZE - 1)   ? GetLightAt(World_LocalXYZ(x, y + 1, z)) : World_LIGHT_LEVEL_SUN,
        (z != 0)                        ? GetLightAt(World_LocalXYZ(x, y, z - 1)) : czn->GetLightAt(World_LocalXYZ(x, y, World_CHUNK_Z_SIZE - 1)),
        (z != World_CHUNK_Z_SIZE - 1)   ? GetLightAt(World_LocalXYZ(x, y, z + 1)) : czp->GetLightAt(World_LocalXYZ(x, y, 0))
    };
}
