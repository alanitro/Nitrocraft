#include "World.hpp"

#include <memory>
#include <unordered_map>
#include "Graphics_Camera.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"
#include "World_ChunkManager.hpp"
#include "World_Generation.hpp"
#include "Utility_Time.hpp"

namespace
{
    constexpr glm::vec3 SKY_COLOR = { 0.2f, 0.75f, 0.95f };

    std::unique_ptr<World_ChunkManager> ChunkManager;
}

void World_Initialize()
{
    ChunkManager = std::make_unique<World_ChunkManager>();
}

void World_Terminate()
{

}

void World_Update(const Camera& camera)
{
    ChunkManager->SetCenterChunkMainThread(World_FromGlobalToChunkID(camera.GetPosition()));
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

    if (chunk == nullptr) return World_Block{ World_Block_ID::AIR };

    return chunk->GetBlockAt(World_FromGlobalToLocal(global));
}

World_Light World_GetLightAt(World_GlobalXYZ global)
{
    auto chunk = World_GetChunkAt(global);

    if (chunk == nullptr) return World_LIGHT_LEVEL_MIN;

    return chunk->GetLightAt(World_FromGlobalToLocal(global));
}

const World_Chunk* World_GetChunkAt(World_GlobalXYZ global)
{
    auto chunk_opt = ChunkManager->GetChunkAt(global);

    if (chunk_opt.has_value()) return chunk_opt.value();

    return nullptr;
}

const World_ChunkManager& World_GetChunkManager()
{
    return *ChunkManager;
}

std::optional<std::pair<World_GlobalXYZ, World_Block_Face>> World_CastRay(glm::vec3 ray_origin, glm::vec3 ray_direction, float ray_length)
{
    assert(ray_direction.x != 0.0f || ray_direction.y != 0.0f || ray_direction.z != 0.0f);
    assert(glm::abs(glm::length(ray_direction) - 1.0f) <= 1e-4f);
    assert(ray_length > 0.0f);

    if (ray_origin.y < 0.0f || ray_origin.y >= static_cast<float>(World_HEIGHT)) return std::nullopt;

    if (World_GetBlockAt(World_GlobalXYZ(ray_origin)) != World_Block(World_Block_ID::AIR)) return std::nullopt;

    World_GlobalXYZ current_voxel_position = World_GlobalXYZ(glm::floor(ray_origin));

    const int step_x = (ray_direction.x > 0.0f) ? 1 : (ray_direction.x < 0.0f) ? -1 : 0;
    const int step_y = (ray_direction.y > 0.0f) ? 1 : (ray_direction.y < 0.0f) ? -1 : 0;
    const int step_z = (ray_direction.z > 0.0f) ? 1 : (ray_direction.z < 0.0f) ? -1 : 0;

    constexpr float EPS = 1e-6f;
    constexpr float INF = std::numeric_limits<float>::infinity();

    const float t_delta_x = (step_x != 0) ? std::abs(1.0f / ray_direction.x) : INF;
    const float t_delta_y = (step_y != 0) ? std::abs(1.0f / ray_direction.y) : INF;
    const float t_delta_z = (step_z != 0) ? std::abs(1.0f / ray_direction.z) : INF;

    float t_max_x = (step_x != 0) ? (((static_cast<float>(current_voxel_position.x) + (step_x == 1 ? 1.0f : 0.0f)) - ray_origin.x) / ray_direction.x) : INF;
    float t_max_y = (step_y != 0) ? (((static_cast<float>(current_voxel_position.y) + (step_y == 1 ? 1.0f : 0.0f)) - ray_origin.y) / ray_direction.y) : INF;
    float t_max_z = (step_z != 0) ? (((static_cast<float>(current_voxel_position.z) + (step_z == 1 ? 1.0f : 0.0f)) - ray_origin.z) / ray_direction.z) : INF;

    float t_traversed = 0.0f;

    while (t_traversed <= ray_length + EPS)
    {
        const float t_next = std::min(t_max_x, std::min(t_max_y, t_max_z));

        World_Block_Face entered_face;

        if (t_max_x <= t_next + EPS)
        {
            current_voxel_position.x += step_x;
            t_max_x += t_delta_x;
            entered_face = step_x == 1 ? World_Block_Face::XN : World_Block_Face::XP;
        }

        if (t_max_y <= t_next + EPS)
        {
            current_voxel_position.y += step_y;
            t_max_y += t_delta_y;
            entered_face = step_y == 1 ? World_Block_Face::YN : World_Block_Face::YP;
        }

        if (t_max_z <= t_next + EPS)
        {
            current_voxel_position.z += step_z;
            t_max_z += t_delta_z;
            entered_face = step_z == 1 ? World_Block_Face::ZN : World_Block_Face::ZP;
        }

        if (World_GetBlockAt(World_GlobalXYZ(current_voxel_position)).ID != World_Block_ID::AIR)
        {
            return std::make_pair(World_GlobalXYZ(current_voxel_position), entered_face);
        }

        if (current_voxel_position.y < 0.0f || current_voxel_position.y >= static_cast<float>(World_HEIGHT)) return std::nullopt;

        t_traversed = t_next;
    }

    return std::nullopt;
}
