#include "World.hpp"

#include <memory>
#include <unordered_map>
#include "Graphics_Camera.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"
#include "World_ActiveArea.hpp"
#include "World_TerrainGeneration.hpp"
#include "Utility_Time.hpp"

namespace
{
    constexpr glm::vec3 SKY_COLOR = { 0.2f, 0.75f, 0.95f };

    std::unordered_map<World_ChunkID, std::unique_ptr<World_Chunk>>         ChunkMap;
    World_ActiveArea   ActiveArea{};
}

void World_Initialize()
{
    World_TerrainGeneration_Initialize(12345);

    World_ActiveArea_LoadChunks(ActiveArea, ChunkMap, World_ChunkID(0, 0, 0));
}

void World_Terminate()
{
    ChunkMap.clear();
}

void World_Update(const Camera& camera)
{
    World_ActiveArea_LoadChunks(ActiveArea, ChunkMap, World_FromGlobalToChunkID(camera.GetPosition()));
}

float World_GetSunlightIntensity()
{
    return std::sin(Time_GetTime()) * 0.5f + 1.0f;
}

glm::vec3 World_GetSkyColor()
{
    return SKY_COLOR * World_GetSunlightIntensity();
}

World_Block World_GetBlockAt(World_GlobalXYZ position)
{
    auto chunk = World_GetChunkAt(position);

    if (chunk == nullptr) return World_Block{ World_BlockID::AIR };

    return World_Chunk_GetBlockAt(chunk, World_FromGlobalToLocal(position));
}

const World_Chunk* World_GetChunkAt(World_GlobalXYZ position)
{
    auto iter = ChunkMap.find(World_FromGlobalToChunkID(position));

    if (iter == ChunkMap.end()) return nullptr;

    return iter->second.get();
}

const World_ActiveArea& World_GetActiveArea()
{
    return ActiveArea;
}
