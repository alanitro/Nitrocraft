#include "World.hpp"

#include <memory>
#include <unordered_map>
#include "Graphics_Camera.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"
#include "World_ChunkManager.hpp"
#include "World_TerrainGenerator.hpp"
#include "Utility_Time.hpp"

void nitrocraft::world::World::Initialize()
{
    m_chunk_manager.Initialize();
}

void nitrocraft::world::World::Terminate()
{
    m_chunk_manager.Terminate();
}

void nitrocraft::world::World::Update(const Camera & camera)
{
    m_chunk_manager.SetCenterChunk_MainThread(FromGlobalToChunkID(camera.GetPosition()));
}

float nitrocraft::world::World::GetSunlightIntensity() const
{
    return std::sin((static_cast<float>(utility::GetTime()))) * 0.5f + 1.0f;
}

glm::vec3 nitrocraft::world::World::GetSkyColor() const
{
    constexpr glm::vec3 SKY_COLOR = { 0.2f, 0.75f, 0.95f };

    return SKY_COLOR * GetSunlightIntensity();
}

nitrocraft::world::Block nitrocraft::world::World::GetBlockAt(GlobalXYZ global) const
{
    if (global.y < 0 || global.y >= HEIGHT) return Block{ BlockID::AIR };

    auto chunk = GetChunkAt(global);

    if (chunk == nullptr) return Block{ BlockID::AIR };

    return chunk->GetBlockAt(FromGlobalToLocal(global));
}

nitrocraft::world::LightLevel nitrocraft::world::World::GetLightAt(GlobalXYZ global) const
{
    if (global.y < 0 || global.y >= HEIGHT) return LIGHT_LEVEL_MIN;

    auto chunk = GetChunkAt(global);

    if (chunk == nullptr) return LIGHT_LEVEL_MIN;

    return chunk->GetLightAt(FromGlobalToLocal(global));
}

const nitrocraft::world::Chunk* nitrocraft::world::World::GetChunkAt(GlobalXYZ global) const
{
    auto chunk_opt = m_chunk_manager.GetChunkAt(global);

    if (chunk_opt.has_value()) return chunk_opt.value();

    return nullptr;
}

const nitrocraft::world::ChunkManager& nitrocraft::world::World::GetChunkManager() const
{
    return m_chunk_manager;
}

void nitrocraft::world::World::SetRenderDistance(std::size_t render_distance)
{
    m_chunk_manager.SetRenderDistance(render_distance);
}

std::optional<nitrocraft::world::RayResult> nitrocraft::world::World::CastRay(glm::vec3 ray_origin, glm::vec3 ray_direction, float ray_length) const
{
    assert(ray_direction.x != 0.0f || ray_direction.y != 0.0f || ray_direction.z != 0.0f);
    assert(glm::abs(glm::length(ray_direction) - 1.0f) <= 1e-4f);
    assert(ray_length > 0.0f);

    if (ray_origin.y < 0.0f || ray_origin.y >= static_cast<float>(HEIGHT)) return std::nullopt;

    if (GetBlockAt(GlobalXYZ(ray_origin)) != Block(BlockID::AIR)) return std::nullopt;

    GlobalXYZ current_voxel_position = GlobalXYZ(glm::floor(ray_origin));

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

        BlockFace entered_face{};

        if (t_max_x <= t_next + EPS)
        {
            current_voxel_position.x += step_x;
            t_max_x += t_delta_x;
            entered_face = step_x == 1 ? BlockFace::XN : BlockFace::XP;
        }

        if (t_max_y <= t_next + EPS)
        {
            current_voxel_position.y += step_y;
            t_max_y += t_delta_y;
            entered_face = step_y == 1 ? BlockFace::YN : BlockFace::YP;
        }

        if (t_max_z <= t_next + EPS)
        {
            current_voxel_position.z += step_z;
            t_max_z += t_delta_z;
            entered_face = step_z == 1 ? BlockFace::ZN : BlockFace::ZP;
        }

        if (GetBlockAt(GlobalXYZ(current_voxel_position)).id != BlockID::AIR)
        {
            return RayResult(GlobalXYZ(current_voxel_position), entered_face);
        }

        if (current_voxel_position.y < 0.0f || current_voxel_position.y >= static_cast<float>(HEIGHT)) return std::nullopt;

        t_traversed = t_next;
    }

    return std::nullopt;
}
