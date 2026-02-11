#include "World_Light.hpp"

#include "World_Coordinate.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"

namespace
{
    void TryAddSunlightAdditionNode(
        std::queue<World_Light_LightAdditionNode>& sunlight_add_queue,
        World_Chunk* chunk, World_LocalXYZ candidate, World_Light source_light
    )
    {
        if (chunk->GetBlockAt(candidate).IsOpaque()) return;

        if (chunk->GetSunlightAt(candidate) + World_LIGHT_LEVEL_02 > source_light) return;

        chunk->SetSunlightAt(candidate, source_light - World_LIGHT_LEVEL_01);

        sunlight_add_queue.emplace(chunk, candidate);
    }

    void TryAddSunlightRemovalNode(
        std::queue<World_Light_LightRemovalNode>& sunlight_rem_queue,
        std::queue<World_Light_LightAdditionNode>& sunlight_add_queue,
        World_Chunk* chunk, World_LocalXYZ candidate, World_Light source_light
    )
    {
        World_Light candidate_light = chunk->GetSunlightAt(candidate);

        if (candidate_light != World_LIGHT_LEVEL_MIN && candidate_light < source_light)
        {
            chunk->SetSunlightAt(candidate, World_LIGHT_LEVEL_MIN);

            sunlight_rem_queue.emplace(chunk, candidate, candidate_light);
        }
        else if (candidate_light >= source_light)
        {
            sunlight_add_queue.emplace(chunk, candidate);
        }
    }

    void TryAddPointlightAdditionNode(
        std::queue<World_Light_LightAdditionNode>& pointlight_add_queue,
        World_Chunk* chunk, World_LocalXYZ candidate, World_Light source_light
    )
    {
        if (chunk->GetBlockAt(candidate).IsOpaque()) return;

        if (chunk->GetPointlightAt(candidate) + World_LIGHT_LEVEL_02 > source_light) return;

        chunk->SetPointlightAt(candidate, source_light - World_LIGHT_LEVEL_01);

        pointlight_add_queue.emplace(chunk, candidate);
    }

    void TryAddPointlightRemovalNode(
        std::queue<World_Light_LightRemovalNode>& pointlight_rem_queue,
        std::queue<World_Light_LightAdditionNode>& pointlight_add_queue,
        World_Chunk* chunk, World_LocalXYZ candidate, World_Light source_light
    )
    {
        World_Light candidate_light = chunk->GetPointlightAt(candidate);

        if (candidate_light != World_LIGHT_LEVEL_MIN && candidate_light < source_light)
        {
            chunk->SetPointlightAt(candidate, World_LIGHT_LEVEL_MIN);

            pointlight_rem_queue.emplace(chunk, candidate, candidate_light);
        }
        else if (candidate_light >= source_light)
        {
            pointlight_add_queue.emplace(chunk, candidate);
        }
    }
}

void World_Light_PropagateSunlight(std::queue<World_Light_LightAdditionNode>& sunlight_add_queue)
{
    while (sunlight_add_queue.empty() == false)
    {
        auto node = sunlight_add_queue.front(); sunlight_add_queue.pop();

        World_Chunk*& chunk = node.Chunk;
        int& lx = node.Local.x;
        int& ly = node.Local.y;
        int& lz = node.Local.z;

        World_Light light = chunk->GetSunlightAt(World_LocalXYZ(lx, ly, lz));

        World_Chunk* cxn = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XNZ0];
        World_Chunk* cxp = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XPZ0];
        World_Chunk* czn = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZN];
        World_Chunk* czp = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZP];

        chunk->HasModified = true;

        // Propagate XN
        if (lx == 0)
            TryAddSunlightAdditionNode(sunlight_add_queue, cxn, World_LocalXYZ(World_CHUNK_X_SIZE - 1, ly, lz), light);
        else
            TryAddSunlightAdditionNode(sunlight_add_queue, chunk, World_LocalXYZ(lx - 1, ly, lz), light);

        // Propagate XP
        if (lx == World_CHUNK_X_SIZE - 1)
            TryAddSunlightAdditionNode(sunlight_add_queue, cxp, World_LocalXYZ(0, ly, lz), light);
        else
            TryAddSunlightAdditionNode(sunlight_add_queue, chunk, World_LocalXYZ(lx + 1, ly, lz), light);

        // Propagate YN
        if (
            ly != 0 &&
            chunk->GetBlockAt(World_LocalXYZ(lx, ly - 1, lz)).IsTransparent() &&
            chunk->GetSunlightAt(World_LocalXYZ(lx, ly - 1, lz)) + World_LIGHT_LEVEL_02 <= light
        )
        {
            chunk->SetSunlightAt(World_LocalXYZ(lx, ly - 1, lz), (light == World_LIGHT_LEVEL_SUN) ? World_LIGHT_LEVEL_SUN : light - World_LIGHT_LEVEL_01);

            sunlight_add_queue.emplace(chunk, World_LocalXYZ(lx, ly - 1, lz));
        }

        // Propagate YP
        if (ly != World_CHUNK_Y_SIZE - 1)
            TryAddSunlightAdditionNode(sunlight_add_queue, chunk, World_LocalXYZ(lx, ly + 1, lz), light);

        // Propagate ZN
        if (lz == 0)
            TryAddSunlightAdditionNode(sunlight_add_queue, czn, World_LocalXYZ(lx, ly, World_CHUNK_Z_SIZE - 1), light);
        else
            TryAddSunlightAdditionNode(sunlight_add_queue, chunk, World_LocalXYZ(lx, ly, lz - 1), light);

        // Propagate ZP
        if (lz == World_CHUNK_Z_SIZE - 1)
            TryAddSunlightAdditionNode(sunlight_add_queue, czp, World_LocalXYZ(lx, ly, 0), light);
        else
            TryAddSunlightAdditionNode(sunlight_add_queue, chunk, World_LocalXYZ(lx, ly, lz + 1), light);
    }
}

void World_Light_UnpropagateSunlight(
    std::queue<World_Light_LightRemovalNode>& sunlight_rem_queue,
    std::queue<World_Light_LightAdditionNode>& sunlight_add_queue
)
{
    while (sunlight_rem_queue.empty() == false)
    {
        auto node = sunlight_rem_queue.front(); sunlight_rem_queue.pop();

        World_Chunk*& chunk = node.Chunk;
        int& lx = node.Local.x;
        int& ly = node.Local.y;
        int& lz = node.Local.z;
        World_Light& light = node.Light;

        World_Chunk* cxn = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XNZ0];
        World_Chunk* cxp = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XPZ0];
        World_Chunk* czn = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZN];
        World_Chunk* czp = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZP];

        chunk->HasModified = true;

        // Unpropagate XN
        if (lx == 0)
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, cxn, World_LocalXYZ(World_CHUNK_X_SIZE - 1, ly, lz), light);
        else
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, chunk, World_LocalXYZ(lx - 1, ly, lz), light);

        // Unpropagate XP
        if (lx == World_CHUNK_X_SIZE - 1)
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, cxp, World_LocalXYZ(0, ly, lz), light);
        else
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, chunk, World_LocalXYZ(lx + 1, ly, lz), light);

        // Unpropagate YN
        if (ly != 0)
        {
            const World_Light yn_light = chunk->GetSunlightAt(World_LocalXYZ(lx, ly - 1, lz));

            const World_LocalXYZ yn_local = World_LocalXYZ(lx, ly - 1, lz);

            if (yn_light == World_LIGHT_LEVEL_SUN)
            {
                chunk->SetSunlightAt(yn_local, World_LIGHT_LEVEL_MIN);

                sunlight_rem_queue.emplace(chunk, yn_local, World_LIGHT_LEVEL_SUN);
            }
            else if (yn_light != World_LIGHT_LEVEL_MIN && yn_light < light)
            {
                chunk->SetSunlightAt(yn_local, World_LIGHT_LEVEL_MIN);

                sunlight_rem_queue.emplace(chunk, yn_local, yn_light);
            }
            else if (yn_light >= light)
            {
                sunlight_add_queue.emplace(chunk, yn_local);
            }
        }

        // Unpropagate YP
        if (ly != World_CHUNK_Y_SIZE - 1)
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, chunk, World_LocalXYZ(lx, ly + 1, lz), light);

        // Unpropagate ZN
        if (lz == 0)
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, czn, World_LocalXYZ(lx, ly, World_CHUNK_Z_SIZE - 1), light);
        else
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, chunk, World_LocalXYZ(lx, ly, lz - 1), light);

        // Unpropagate ZP
        if (lz == World_CHUNK_Z_SIZE - 1)
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, czp, World_LocalXYZ(lx, ly, 0), light);
        else
            TryAddSunlightRemovalNode(sunlight_rem_queue, sunlight_add_queue, chunk, World_LocalXYZ(lx, ly, lz + 1), light);
    }

    // Fill in the gap of removed sunlight
    World_Light_PropagateSunlight(sunlight_add_queue);
}

void World_Light_PropagatePointlight(std::queue<World_Light_LightAdditionNode>& pointlight_add_queue)
{
    while (pointlight_add_queue.empty() == false)
    {
        auto node = pointlight_add_queue.front(); pointlight_add_queue.pop();

        World_Chunk*& chunk = node.Chunk;
        int& lx = node.Local.x;
        int& ly = node.Local.y;
        int& lz = node.Local.z;

        World_Light light = chunk->GetPointlightAt(World_LocalXYZ(lx, ly, lz));

        World_Chunk* cxn = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XNZ0];
        World_Chunk* cxp = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XPZ0];
        World_Chunk* czn = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZN];
        World_Chunk* czp = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZP];

        chunk->HasModified = true;

        // Propagate XN
        if (lx == 0)
            TryAddPointlightAdditionNode(pointlight_add_queue, cxn, World_LocalXYZ(World_CHUNK_X_SIZE - 1, ly, lz), light);
        else
            TryAddPointlightAdditionNode(pointlight_add_queue, chunk, World_LocalXYZ(lx - 1, ly, lz), light);

        // Propagate XP
        if (lx == World_CHUNK_X_SIZE - 1)
            TryAddPointlightAdditionNode(pointlight_add_queue, cxp, World_LocalXYZ(0, ly, lz), light);
        else
            TryAddPointlightAdditionNode(pointlight_add_queue, chunk, World_LocalXYZ(lx + 1, ly, lz), light);

        // Propagate YN
        if (ly != 0)
            TryAddPointlightAdditionNode(pointlight_add_queue, chunk, World_LocalXYZ(lx, ly - 1, lz), light);

        // Propagate YP
        if (ly != World_CHUNK_Y_SIZE - 1)
            TryAddPointlightAdditionNode(pointlight_add_queue, chunk, World_LocalXYZ(lx, ly + 1, lz), light);

        // Propagate ZN
        if (lz == 0)
            TryAddPointlightAdditionNode(pointlight_add_queue, czn, World_LocalXYZ(lx, ly, World_CHUNK_Z_SIZE - 1), light);
        else
            TryAddPointlightAdditionNode(pointlight_add_queue, chunk, World_LocalXYZ(lx, ly, lz - 1), light);

        // Propagate ZP
        if (lz == World_CHUNK_Z_SIZE - 1)
            TryAddPointlightAdditionNode(pointlight_add_queue, czp, World_LocalXYZ(lx, ly, 0), light);
        else
            TryAddPointlightAdditionNode(pointlight_add_queue, chunk, World_LocalXYZ(lx, ly, lz + 1), light);
    }
}

void World_Light_UnpropagatePointlight(
    std::queue<World_Light_LightRemovalNode>& pointlight_rem_queue,
    std::queue<World_Light_LightAdditionNode>& pointlight_add_queue
)
{
    while (pointlight_rem_queue.empty() == false)
    {
        auto node = pointlight_rem_queue.front(); pointlight_rem_queue.pop();

        World_Chunk*& chunk = node.Chunk;
        World_Light& light = node.Light;
        int& lx = node.Local.x;
        int& ly = node.Local.y;
        int& lz = node.Local.z;

        World_Chunk* cxn = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XNZ0];
        World_Chunk* cxp = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::XPZ0];
        World_Chunk* czn = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZN];
        World_Chunk* czp = chunk->Neighbours[(std::size_t)World_Chunk_Neighbour::X0ZP];

        chunk->HasModified = true;

        // Unpropagate XN
        if (lx == 0)
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, cxn, World_LocalXYZ(World_CHUNK_X_SIZE - 1, ly, lz), light);
        else
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, chunk, World_LocalXYZ(lx - 1, ly, lz), light);

        // Unpropagate XP
        if (lx == World_CHUNK_X_SIZE - 1)
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, cxp, World_LocalXYZ(0, ly, lz), light);
        else
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, chunk, World_LocalXYZ(lx + 1, ly, lz), light);

        // Unpropagate YN
        if (ly != 0)
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, chunk, World_LocalXYZ(lx, ly - 1, lz), light);

        // Unpropagate YP
        if (ly != World_CHUNK_Y_SIZE - 1)
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, chunk, World_LocalXYZ(lx, ly + 1, lz), light);

        // Unpropagate ZN
        if (lz == 0)
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, czn, World_LocalXYZ(lx, ly, World_CHUNK_Z_SIZE - 1), light);
        else
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, chunk, World_LocalXYZ(lx, ly, lz - 1), light);

        // Unpropagate ZP
        if (lz == World_CHUNK_Z_SIZE - 1)
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, czp, World_LocalXYZ(lx, ly, 0), light);
        else
            TryAddPointlightRemovalNode(pointlight_rem_queue, pointlight_add_queue, chunk, World_LocalXYZ(lx, ly, lz + 1), light);
    }

    // Fill in the gap of removed pointlight
    World_Light_PropagatePointlight(pointlight_add_queue);
}
