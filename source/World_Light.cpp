#include "World_Light.hpp"

#include <algorithm>
#include <print> // TODO: remove
#include "World_Coordinate.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"

namespace nitrocraft::world
{

namespace
{
    void TryAddSunlightAdditionNode(
        std::queue<LightAdditionNode>& sunlight_add_queue,
        Chunk* chunk, LocalXYZ candidate, LightLevel source_light
    )
    {
        if (chunk->GetBlockAt(candidate).IsOpaque()) return;

        if (chunk->GetSunlightAt(candidate) + LIGHT_LEVEL_02 > source_light) return;

        chunk->SetSunlightAt(candidate, source_light - LIGHT_LEVEL_01);

        sunlight_add_queue.emplace(chunk, candidate);
    }

    void TryAddSunlightRemovalNode(
        std::queue<LightRemovalNode>& sunlight_rem_queue,
        std::queue<LightAdditionNode>& sunlight_add_queue,
        Chunk* chunk, LocalXYZ candidate, LightLevel source_light
    )
    {
        LightLevel candidate_light = chunk->GetSunlightAt(candidate);

        if (candidate_light != LIGHT_LEVEL_MIN && candidate_light < source_light)
        {
            chunk->SetSunlightAt(candidate, LIGHT_LEVEL_MIN);

            sunlight_rem_queue.emplace(chunk, candidate, candidate_light);
        }
        else if (candidate_light >= source_light)
        {
            sunlight_add_queue.emplace(chunk, candidate);
        }
    }

    void TryAddPointlightAdditionNode(
        std::queue<LightAdditionNode>& pointlight_add_queue,
        Chunk* chunk, LocalXYZ candidate, LightLevel source_light
    )
    {
        if (chunk->GetBlockAt(candidate).IsOpaque()) return;

        if (chunk->GetPointlightAt(candidate) + LIGHT_LEVEL_02 > source_light) return;

        chunk->SetPointlightAt(candidate, source_light - LIGHT_LEVEL_01);

        pointlight_add_queue.emplace(chunk, candidate);
    }

    void TryAddPointlightRemovalNode(
        std::queue<LightRemovalNode>& pointlight_rem_queue,
        std::queue<LightAdditionNode>& pointlight_add_queue,
        Chunk* chunk, LocalXYZ candidate, LightLevel source_light
    )
    {
        LightLevel candidate_light = chunk->GetPointlightAt(candidate);

        if (candidate_light != LIGHT_LEVEL_MIN && candidate_light < source_light)
        {
            chunk->SetPointlightAt(candidate, LIGHT_LEVEL_MIN);

            pointlight_rem_queue.emplace(chunk, candidate, candidate_light);
        }
        else if (candidate_light >= source_light)
        {
            pointlight_add_queue.emplace(chunk, candidate);
        }
    }
}

void PropagateSunlight(std::queue<LightAdditionNode>& sunlight_add_queue)
{
    while (sunlight_add_queue.empty() == false)
    {
        auto node = sunlight_add_queue.front(); sunlight_add_queue.pop();

        Chunk*& chunk = node.chunk;
        int& lx = node.local.x;
        int& ly = node.local.y;
        int& lz = node.local.z;

        LightLevel light = chunk->GetSunlightAt(LocalXYZ(lx, ly, lz));

        // TODO: remove
        if (chunk->neighbours_set.load(std::memory_order_acquire) == false)
        {
            std::println("{} {}: neighbours not set", chunk->id.x, chunk->id.z);
            return;
        }

        Chunk* cxn = chunk->neighbours[(std::size_t)ChunkNeighbour::XNZ0];
        Chunk* cxp = chunk->neighbours[(std::size_t)ChunkNeighbour::XPZ0];
        Chunk* czn = chunk->neighbours[(std::size_t)ChunkNeighbour::X0ZN];
        Chunk* czp = chunk->neighbours[(std::size_t)ChunkNeighbour::X0ZP];

        chunk->has_modified = true;

        // Propagate XN
        if (lx == 0)
            TryAddSunlightAdditionNode(sunlight_add_queue, cxn, LocalXYZ(CHUNK_X_SIZE - 1, ly, lz), light);
        else
            TryAddSunlightAdditionNode(sunlight_add_queue, chunk, LocalXYZ(lx - 1, ly, lz), light);

        // Propagate XP
        if (lx == CHUNK_X_SIZE - 1)
            TryAddSunlightAdditionNode(sunlight_add_queue, cxp, LocalXYZ(0, ly, lz), light);
        else
            TryAddSunlightAdditionNode(sunlight_add_queue, chunk, LocalXYZ(lx + 1, ly, lz), light);

        // Propagate YN
        if (
            ly != 0 &&
            chunk->GetBlockAt(LocalXYZ(lx, ly - 1, lz)).IsTransparent() &&
            chunk->GetSunlightAt(LocalXYZ(lx, ly - 1, lz)) + LIGHT_LEVEL_02 <= light
        )
        {
            chunk->SetSunlightAt(LocalXYZ(lx, ly - 1, lz), (light == LIGHT_LEVEL_SUN) ? LIGHT_LEVEL_SUN : light - LIGHT_LEVEL_01);

            sunlight_add_queue.emplace(chunk, LocalXYZ(lx, ly - 1, lz));
        }

        // Propagate YP
        if (ly != CHUNK_Y_SIZE - 1)
            TryAddSunlightAdditionNode(sunlight_add_queue, chunk, LocalXYZ(lx, ly + 1, lz), light);

        // Propagate ZN
        if (lz == 0)
            TryAddSunlightAdditionNode(sunlight_add_queue, czn, LocalXYZ(lx, ly, CHUNK_Z_SIZE - 1), light);
        else
            TryAddSunlightAdditionNode(sunlight_add_queue, chunk, LocalXYZ(lx, ly, lz - 1), light);

        // Propagate ZP
        if (lz == CHUNK_Z_SIZE - 1)
            TryAddSunlightAdditionNode(sunlight_add_queue, czp, LocalXYZ(lx, ly, 0), light);
        else
            TryAddSunlightAdditionNode(sunlight_add_queue, chunk, LocalXYZ(lx, ly, lz + 1), light);
    }
}

void UnpropagateSunlight(
    std::queue<LightRemovalNode>& sunlight_rem_queue,
    std::queue<LightAdditionNode>& sunlight_add_queue
)
{
    while (sunlight_rem_queue.empty() == false)
    {
        auto node = sunlight_rem_queue.front(); sunlight_rem_queue.pop();

        Chunk*& chunk = node.chunk;
        int& lx = node.local.x;
        int& ly = node.local.y;
        int& lz = node.local.z;
        LightLevel& light = node.light;

        Chunk* cxn = chunk->neighbours[(std::size_t)ChunkNeighbour::XNZ0];
        Chunk* cxp = chunk->neighbours[(std::size_t)ChunkNeighbour::XPZ0];
        Chunk* czn = chunk->neighbours[(std::size_t)ChunkNeighbour::X0ZN];
        Chunk* czp = chunk->neighbours[(std::size_t)ChunkNeighbour::X0ZP];

        chunk->has_modified = true;

        // Unpropagate XN
        if (lx == 0)
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, cxn, LocalXYZ(CHUNK_X_SIZE - 1, ly, lz), light);
        else
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, chunk, LocalXYZ(lx - 1, ly, lz), light);

        // Unpropagate XP
        if (lx == CHUNK_X_SIZE - 1)
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, cxp, LocalXYZ(0, ly, lz), light);
        else
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, chunk, LocalXYZ(lx + 1, ly, lz), light);

        // Unpropagate YN
        if (ly != 0)
        {
            const LightLevel yn_light = chunk->GetSunlightAt(LocalXYZ(lx, ly - 1, lz));

            const LocalXYZ yn_local = LocalXYZ(lx, ly - 1, lz);

            if (yn_light == LIGHT_LEVEL_SUN)
            {
                chunk->SetSunlightAt(yn_local, LIGHT_LEVEL_MIN);

                sunlight_rem_queue.emplace(chunk, yn_local, LIGHT_LEVEL_SUN);
            }
            else if (yn_light != LIGHT_LEVEL_MIN && yn_light < light)
            {
                chunk->SetSunlightAt(yn_local, LIGHT_LEVEL_MIN);

                sunlight_rem_queue.emplace(chunk, yn_local, yn_light);
            }
            else if (yn_light >= light)
            {
                sunlight_add_queue.emplace(chunk, yn_local);
            }
        }

        // Unpropagate YP
        if (ly != CHUNK_Y_SIZE - 1)
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, chunk, LocalXYZ(lx, ly + 1, lz), light);

        // Unpropagate ZN
        if (lz == 0)
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, czn, LocalXYZ(lx, ly, CHUNK_Z_SIZE - 1), light);
        else
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, chunk, LocalXYZ(lx, ly, lz - 1), light);

        // Unpropagate ZP
        if (lz == CHUNK_Z_SIZE - 1)
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, czp, LocalXYZ(lx, ly, 0), light);
        else
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, chunk, LocalXYZ(lx, ly, lz + 1), light);
    }

    // Fill in the gap of removed sunlight
    PropagateSunlight(sunlight_add_queue);
}

void PropagatePointlight(std::queue<LightAdditionNode>& pointlight_add_queue)
{
    while (pointlight_add_queue.empty() == false)
    {
        auto node = pointlight_add_queue.front(); pointlight_add_queue.pop();

        Chunk*& chunk = node.chunk;
        int& lx = node.local.x;
        int& ly = node.local.y;
        int& lz = node.local.z;

        LightLevel light = chunk->GetPointlightAt(LocalXYZ(lx, ly, lz));

        Chunk* cxn = chunk->neighbours[(std::size_t)ChunkNeighbour::XNZ0];
        Chunk* cxp = chunk->neighbours[(std::size_t)ChunkNeighbour::XPZ0];
        Chunk* czn = chunk->neighbours[(std::size_t)ChunkNeighbour::X0ZN];
        Chunk* czp = chunk->neighbours[(std::size_t)ChunkNeighbour::X0ZP];

        chunk->has_modified = true;

        // Propagate XN
        if (lx == 0)
            TryAddPointlightAdditionNode(pointlight_add_queue, cxn, LocalXYZ(CHUNK_X_SIZE - 1, ly, lz), light);
        else
            TryAddPointlightAdditionNode(pointlight_add_queue, chunk, LocalXYZ(lx - 1, ly, lz), light);

        // Propagate XP
        if (lx == CHUNK_X_SIZE - 1)
            TryAddPointlightAdditionNode(pointlight_add_queue, cxp, LocalXYZ(0, ly, lz), light);
        else
            TryAddPointlightAdditionNode(pointlight_add_queue, chunk, LocalXYZ(lx + 1, ly, lz), light);

        // Propagate YN
        if (ly != 0)
            TryAddPointlightAdditionNode(pointlight_add_queue, chunk, LocalXYZ(lx, ly - 1, lz), light);

        // Propagate YP
        if (ly != CHUNK_Y_SIZE - 1)
            TryAddPointlightAdditionNode(pointlight_add_queue, chunk, LocalXYZ(lx, ly + 1, lz), light);

        // Propagate ZN
        if (lz == 0)
            TryAddPointlightAdditionNode(pointlight_add_queue, czn, LocalXYZ(lx, ly, CHUNK_Z_SIZE - 1), light);
        else
            TryAddPointlightAdditionNode(pointlight_add_queue, chunk, LocalXYZ(lx, ly, lz - 1), light);

        // Propagate ZP
        if (lz == CHUNK_Z_SIZE - 1)
            TryAddPointlightAdditionNode(pointlight_add_queue, czp, LocalXYZ(lx, ly, 0), light);
        else
            TryAddPointlightAdditionNode(pointlight_add_queue, chunk, LocalXYZ(lx, ly, lz + 1), light);
    }
}

void UnpropagatePointlight(
    std::queue<LightRemovalNode>& pointlight_rem_queue,
    std::queue<LightAdditionNode>& pointlight_add_queue
)
{
    while (pointlight_rem_queue.empty() == false)
    {
        auto node = pointlight_rem_queue.front(); pointlight_rem_queue.pop();

        Chunk*& chunk = node.chunk;
        LightLevel& light = node.light;
        int& lx = node.local.x;
        int& ly = node.local.y;
        int& lz = node.local.z;

        Chunk* cxn = chunk->neighbours[(std::size_t)ChunkNeighbour::XNZ0];
        Chunk* cxp = chunk->neighbours[(std::size_t)ChunkNeighbour::XPZ0];
        Chunk* czn = chunk->neighbours[(std::size_t)ChunkNeighbour::X0ZN];
        Chunk* czp = chunk->neighbours[(std::size_t)ChunkNeighbour::X0ZP];

        chunk->has_modified = true;

        // Unpropagate XN
        if (lx == 0)
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, cxn, LocalXYZ(CHUNK_X_SIZE - 1, ly, lz), light);
        else
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, chunk, LocalXYZ(lx - 1, ly, lz), light);

        // Unpropagate XP
        if (lx == CHUNK_X_SIZE - 1)
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, cxp, LocalXYZ(0, ly, lz), light);
        else
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, chunk, LocalXYZ(lx + 1, ly, lz), light);

        // Unpropagate YN
        if (ly != 0)
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, chunk, LocalXYZ(lx, ly - 1, lz), light);

        // Unpropagate YP
        if (ly != CHUNK_Y_SIZE - 1)
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, chunk, LocalXYZ(lx, ly + 1, lz), light);

        // Unpropagate ZN
        if (lz == 0)
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, czn, LocalXYZ(lx, ly, CHUNK_Z_SIZE - 1), light);
        else
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, chunk, LocalXYZ(lx, ly, lz - 1), light);

        // Unpropagate ZP
        if (lz == CHUNK_Z_SIZE - 1)
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, czp, LocalXYZ(lx, ly, 0), light);
        else
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, chunk, LocalXYZ(lx, ly, lz + 1), light);
    }

    // Fill in the gap of removed pointlight
    PropagatePointlight(pointlight_add_queue);
}

void PropagateInitialSunlight(Chunk* chunk)
{
    std::queue<LightAdditionNode> sunlight_add_queue;

    int max_height = chunk->GetMaxHeight();

    for (auto n : chunk->neighbours)
    {
        auto n_max = n->GetMaxHeight();
        if (n_max > max_height) max_height = n_max;
    }

    for (int lz = 0; lz < CHUNK_Z_SIZE; lz++)
    for (int lx = 0; lx < CHUNK_X_SIZE; lx++)
    for (int ly = CHUNK_Y_SIZE - 1; chunk->GetBlockAt(LocalXYZ(lx, ly, lz)).IsOpaque() == false && ly >= 0; ly--)
    {
        chunk->SetSunlightAt(LocalXYZ(lx, ly, lz), LIGHT_LEVEL_SUN);

        if (ly <= max_height) sunlight_add_queue.emplace(chunk, LocalXYZ(lx, ly, lz));
    }

    PropagateSunlight(sunlight_add_queue);
}

} // namespace nitrocraft::world
