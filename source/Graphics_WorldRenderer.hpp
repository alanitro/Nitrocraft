#pragma once

#include <utility>
#include <vector>
#include <thread>
#include <atomic>
#include <condition_variable>
#include "Graphics_Shader.hpp"
#include "World_Coordinate.hpp"
#include "World_ChunkManager.hpp"

class Camera;
struct World_Chunk;

class Graphics_WorldRenderer
{
public:
    Graphics_WorldRenderer() = default;
    ~Graphics_WorldRenderer() = default;

    void Initialize();
    void Terminate();

    void Render(const Camera& camera, float sunlight_intensity, glm::vec3 sky_color);

    void PrepareChunksToRender(const std::vector<World_Chunk*>& chunks_in_render_area);

private:
    // Graphics Pipeline
    GLuint m_BlockTextureAtlas = 0;

    Graphics_Shader m_ChunkShader;

    // Mesh storage
    std::vector<World_Chunk_ID> m_GPUMeshIDsToRender;
    std::unordered_map<World_Chunk_ID, std::unique_ptr<Graphics_ChunkGPUMeshHandle>> m_ChunkGPUMeshHandles;

    // Meshing threads
    std::vector<std::jthread>  m_MeshingThreads;
    std::size_t                m_MeshingThreadCount = 0;
};
