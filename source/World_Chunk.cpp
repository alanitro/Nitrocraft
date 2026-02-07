#include "World_Chunk.hpp"

World_Block World_Chunk_GetBlockAt(const World_Chunk* self, World_LocalXYZ local)
{
    return self->Payload->Blocks.At(local.x, local.y, local.z);
}

World_Light World_Chunk_GetLightAt(const World_Chunk* self, World_LocalXYZ local)
{
    return self->Payload->Lights.At(local.x, local.y, local.z);
}

World_Light World_Chunk_GetSunlightAt(const World_Chunk* self, World_LocalXYZ local)
{
    return World_ExtractSunlight(self->Payload->Lights.At(local.x, local.y, local.z));
}

World_Light World_Chunk_GetPointlightAt(const World_Chunk* self, World_LocalXYZ local)
{
    return World_ExtractPointlight(self->Payload->Lights.At(local.x, local.y, local.z));
}

void World_Chunk_SetBlockAt(World_Chunk* self, World_LocalXYZ local, World_Block block)
{
    self->Payload->Blocks.At(local.x, local.y, local.z) = block;
}

void World_Chunk_SetLightAt(World_Chunk* self, World_LocalXYZ local, World_Light sunlight, World_Light pointlight)
{
    self->Payload->Lights.At(local.x, local.y, local.z) = ((sunlight << 0) & 0x0F) | ((pointlight << 4) & 0xF0);
}

void World_Chunk_SetSunlightAt(World_Chunk* self, World_LocalXYZ local, World_Light sunlight)
{
    auto& light = self->Payload->Lights.At(local.x, local.y, local.z);

    light = (light & 0xF0) | ((sunlight << 0) & 0x0F);
}

void World_Chunk_SetPointlightAt(World_Chunk* self, World_LocalXYZ local, World_Light pointlight)
{
    auto& light = self->Payload->Lights.At(local.x, local.y, local.z);

    light = (light & 0x0F) | ((pointlight << 4) & 0xF0);
}

int World_Chunk_GetHeightAt(const World_Chunk* self, int local_x, int local_z)
{
    return self->Payload->Heights.At(local_x, local_z);
}

void World_Chunk_SetHeightAt(World_Chunk* self, int local_x, int local_z, int height)
{
    self->Payload->Heights.At(local_x, local_z) = static_cast<std::uint8_t>(height);
}

std::array<World_Block, static_cast<std::size_t>(World_BlockNeighbour::COUNT)> World_Chunk_GetNeighbourBlocksAt(const World_Chunk* self, World_LocalXYZ local)
{
    int& x = local.x;
    int& y = local.y;
    int& z = local.z;

    World_Chunk* cxn = self->NeighbourXNZ0;
    World_Chunk* cxp = self->NeighbourXPZ0;
    World_Chunk* czn = self->NeighbourX0ZN;
    World_Chunk* czp = self->NeighbourX0ZP;

    return std::array<World_Block, static_cast<std::size_t>(World_BlockNeighbour::COUNT)>
    {
        (x != 0)                        ? World_Chunk_GetBlockAt(self, World_LocalXYZ(x - 1, y, z)) : World_Chunk_GetBlockAt(cxn, World_LocalXYZ(World_CHUNK_X_SIZE - 1, y, z)),
        (x != World_CHUNK_X_SIZE - 1)   ? World_Chunk_GetBlockAt(self, World_LocalXYZ(x + 1, y, z)) : World_Chunk_GetBlockAt(cxp, World_LocalXYZ(0, y, z)),
        (y != 0)                        ? World_Chunk_GetBlockAt(self, World_LocalXYZ(x, y - 1, z)) : World_Block(World_BlockID::AIR),
        (y != World_CHUNK_Y_SIZE - 1)   ? World_Chunk_GetBlockAt(self, World_LocalXYZ(x, y + 1, z)) : World_Block(World_BlockID::AIR),
        (z != 0)                        ? World_Chunk_GetBlockAt(self, World_LocalXYZ(x, y, z - 1)) : World_Chunk_GetBlockAt(czn, World_LocalXYZ(x, y, World_CHUNK_Z_SIZE - 1)),
        (z != World_CHUNK_Z_SIZE - 1)   ? World_Chunk_GetBlockAt(self, World_LocalXYZ(x, y, z + 1)) : World_Chunk_GetBlockAt(czp, World_LocalXYZ(x, y, 0))
    };
}

std::array<World_Light, static_cast<std::size_t>(World_BlockNeighbour::COUNT)> World_Chunk_GetNeighbourLightsAt(const World_Chunk* self, World_LocalXYZ local)
{
    int& x = local.x;
    int& y = local.y;
    int& z = local.z;

    World_Chunk* cxn = self->NeighbourXNZ0;
    World_Chunk* cxp = self->NeighbourXPZ0;
    World_Chunk* czn = self->NeighbourX0ZN;
    World_Chunk* czp = self->NeighbourX0ZP;

    return std::array<World_Light, static_cast<std::size_t>(World_BlockNeighbour::COUNT)>
    {
        (x != 0)                        ? World_Chunk_GetLightAt(self, World_LocalXYZ(x - 1, y, z)) : World_Chunk_GetLightAt(cxn, World_LocalXYZ(World_CHUNK_X_SIZE - 1, y, z)),
        (x != World_CHUNK_X_SIZE - 1)   ? World_Chunk_GetLightAt(self, World_LocalXYZ(x + 1, y, z)) : World_Chunk_GetLightAt(cxp, World_LocalXYZ(0, y, z)),
        (y != 0)                        ? World_Chunk_GetLightAt(self, World_LocalXYZ(x, y - 1, z)) : World_LIGHT_LEVEL_MIN,
        (y != World_CHUNK_Y_SIZE - 1)   ? World_Chunk_GetLightAt(self, World_LocalXYZ(x, y + 1, z)) : World_LIGHT_LEVEL_SUN,
        (z != 0)                        ? World_Chunk_GetLightAt(self, World_LocalXYZ(x, y, z - 1)) : World_Chunk_GetLightAt(czn, World_LocalXYZ(x, y, World_CHUNK_Z_SIZE - 1)),
        (z != World_CHUNK_Z_SIZE - 1)   ? World_Chunk_GetLightAt(self, World_LocalXYZ(x, y, z + 1)) : World_Chunk_GetLightAt(czp, World_LocalXYZ(x, y, 0))
    };
}
