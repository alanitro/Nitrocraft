#include "World.hpp"

#include <memory>
#include <unordered_map>
#include "Graphics_Camera.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"
#include "World_ActiveArea.hpp"
#include "World_TerrainGeneration.hpp"
#include "Utility_Time.hpp"

#include <print> // todo remove

namespace
{
    constexpr glm::vec3 SKY_COLOR = { 0.2f, 0.75f, 0.95f };

    std::unordered_map<World_ChunkID, std::unique_ptr<World_Chunk>>         ChunkMap;
    World_ActiveArea                                ActiveArea{};
    std::queue<World_Lighting_LightAdditionNode>    SunlightAdditionBFSQueue;
    std::queue<World_Lighting_LightRemovalNode>     SunlightRemovalBFSQueue;
    std::queue<World_Lighting_LightAdditionNode>    PointlightAdditionBFSQueue;
    std::queue<World_Lighting_LightRemovalNode>     PointlightRemovalBFSQueue;
}

void World_Initialize()
{
    World_TerrainGeneration_Initialize(12345);

    World_ActiveArea_LoadChunks(ActiveArea, ChunkMap, World_LocalXYZ(0, 0, 0));

    World_ActiveArea_PerformGenerationPhase(ActiveArea);

    World_ActiveArea_PerformLightingPhase(
        ActiveArea,
        SunlightAdditionBFSQueue,
        SunlightRemovalBFSQueue,
        PointlightAdditionBFSQueue,
        PointlightRemovalBFSQueue
    );
}

void World_Terminate()
{
    ChunkMap.clear();
}

void World_Update(const Camera& camera)
{
    World_ActiveArea_LoadChunks(ActiveArea, ChunkMap, World_FromGlobalToChunkID(camera.GetPosition()));

    World_ActiveArea_PerformGenerationPhase(ActiveArea);

    World_ActiveArea_PerformLightingPhase(
        ActiveArea,
        SunlightAdditionBFSQueue,
        SunlightRemovalBFSQueue,
        PointlightAdditionBFSQueue,
        PointlightRemovalBFSQueue
    );
}

float World_GetSunlightIntensity()
{
    return std::sin((float)Time_GetTime()) * 0.5f + 1.0f;
}

glm::vec3 World_GetSkyColor()
{
    return SKY_COLOR * World_GetSunlightIntensity();
}

World_Block World_GetBlockAt(World_GlobalXYZ global)
{
    auto chunk = World_GetChunkAt(global);

    if (chunk == nullptr) return World_Block{ World_BlockID::AIR };

    return World_Chunk_GetBlockAt(chunk, World_FromGlobalToLocal(global));
}

World_Light World_GetLightAt(World_GlobalXYZ global)
{
    auto chunk = World_GetChunkAt(global);

    if (chunk == nullptr) return World_LIGHT_LEVEL_MIN;

    return World_Chunk_GetLightAt(chunk, World_FromGlobalToLocal(global));
}

const World_Chunk* World_GetChunkAt(World_GlobalXYZ global)
{
    auto iter = ChunkMap.find(World_FromGlobalToChunkID(global));

    if (iter == ChunkMap.end()) return nullptr;

    return iter->second.get();
}

const World_ActiveArea& World_GetActiveArea()
{
    return ActiveArea;
}
