#pragma once

#include <utility>
#include <optional>
#include "World_Coordinate.hpp"
#include "World_ChunkManager.hpp"
#include "World_Block.hpp"
#include "Utility_Array2D.hpp"

class Camera;
struct World_Chunk;

namespace nitrocraft::world
{

struct RayResult
{
    GlobalXYZ position;
    BlockFace face;
};

class World
{
public:
    void Initialize();
    void Terminate();
    void Update(const Camera& camera);

    float GetSunlightIntensity() const;
    glm::vec3 GetSkyColor() const;

    Block GetBlockAt(GlobalXYZ global) const;
    LightLevel GetLightAt(GlobalXYZ global) const;

    const Chunk* GetChunkAt(GlobalXYZ global) const;

    const ChunkManager& GetChunkManager() const;

    void SetRenderDistance(std::size_t render_distance);

    std::optional<RayResult> CastRay(glm::vec3 ray_origin, glm::vec3 ray_direction, float ray_length) const;

private:
    ChunkManager m_chunk_manager;
};

} // namespace nitrocraft::world
