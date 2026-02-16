#include "Graphics_WorldRenderer.hpp"

#include <array>
#include <unordered_map>
#include <print>
#include <glm/vec2.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/gl.h>
#include "Utility_IO.hpp"
#include "Graphics_Camera.hpp"
#include "World_Coordinate.hpp"
#include "World_Block.hpp"
#include "World_Chunk.hpp"
#include "Utility_Time.hpp"

namespace
{
    struct ChunkGPUMeshHandle
    {
        GLuint VertexArrayID;
        GLuint VertexBufferID;
        GLuint IndexBufferID;
        std::uint32_t IndicesCount;
    };

    GLuint ChunkShaderProgram = 0;
    GLuint BlocksTexture = 0;


    std::vector<World_ChunkID> GPUMeshIDsToRender;
    std::unordered_map<World_ChunkID, ChunkGPUMeshHandle> GPUMeshHandleMap;

    void UploadCPUMeshToGPU(World_Chunk* chunk);
    void LoadShaderProgram();
    void LoadTexture();

} // !namespace internal

void WorldRenderer_Initialize()
{
    LoadShaderProgram();

    LoadTexture();
}

void WorldRenderer_Terminate()
{
    for (auto& [chunk_id, chunk_mesh] : GPUMeshHandleMap)
    {
        glDeleteVertexArrays(1, &chunk_mesh.VertexArrayID);
        glDeleteBuffers(1, &chunk_mesh.VertexBufferID);
        glDeleteBuffers(1, &chunk_mesh.IndexBufferID);
        chunk_mesh.IndicesCount = 0;
    }

    glDeleteProgram(ChunkShaderProgram);

    glDeleteTextures(1, &BlocksTexture);
}

void WorldRenderer_Render(const Camera& camera, float sunlight_intensity, glm::vec3 sky_color)
{
    glClearColor(sky_color.r, sky_color.g, sky_color.b, 1.0f);

    glUseProgram(ChunkShaderProgram);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, BlocksTexture);
    glUniform1i(glGetUniformLocation(ChunkShaderProgram, "u_Texture"), 0);

    auto& model_view_projection = camera.GetViewProjection();

    glUniformMatrix4fv(glGetUniformLocation(ChunkShaderProgram, "u_ModelViewProjection"), 1, GL_FALSE, glm::value_ptr(model_view_projection));
    
    glUniform1f(glGetUniformLocation(ChunkShaderProgram, "u_SunlightIntensity"), sunlight_intensity);

    for (auto& chunk_id : GPUMeshIDsToRender)
    {
        auto it = GPUMeshHandleMap.find(chunk_id);

        if (it == GPUMeshHandleMap.end()) continue;
        
        auto& handle = it->second;

        glBindVertexArray(handle.VertexArrayID);

        glDrawElements(GL_TRIANGLES, handle.IndicesCount, GL_UNSIGNED_INT, reinterpret_cast<const void*>(0));
    }

    GPUMeshIDsToRender.clear();
}

void WorldRenderer_PrepareChunksToRender(const std::vector<World_Chunk*>& chunks_in_render_area)
{
    for (auto c : chunks_in_render_area)
    {
        UploadCPUMeshToGPU(c);
    }
}

namespace // internal
{
    void UploadCPUMeshToGPU(World_Chunk* chunk)
    {
        World_ChunkID chunk_id = chunk->ID;
        World_GlobalXYZ chunk_offset = World_FromChunkIDToChunkOffset(chunk_id);

        // If the chunk has completed cpu mesh -> delete the old gpu mesh handle, if it exists.
        // If the chunk does not have completed cpu mesh -> use the old gpu mesh handle, if it exists, exit otherwise.
        World_Chunk_Stage expected = World_Chunk_Stage::MeshingComplete;
        if (chunk->Stage.compare_exchange_strong(expected, World_Chunk_Stage::Ready, std::memory_order_acq_rel, std::memory_order_acquire))
        {
            if (auto iter = GPUMeshHandleMap.find(chunk_id); iter != GPUMeshHandleMap.end())
            {
                glDeleteVertexArrays(1, &iter->second.VertexArrayID);
                glDeleteBuffers(1, &iter->second.VertexBufferID);
                glDeleteBuffers(1, &iter->second.IndexBufferID);
                iter->second.IndicesCount = 0;
                GPUMeshHandleMap.erase(chunk_id);
            }

            // TODO: use buffer orphaning instead of recreating new buffer
            // Create new gpu mesh handle.
            ChunkGPUMeshHandle gpumesh_handle;
            glGenVertexArrays(1, &gpumesh_handle.VertexArrayID);
            glGenBuffers(1, &gpumesh_handle.VertexBufferID);
            glGenBuffers(1, &gpumesh_handle.IndexBufferID);

            glBindVertexArray(gpumesh_handle.VertexArrayID);
            glBindBuffer(GL_ARRAY_BUFFER, gpumesh_handle.VertexBufferID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpumesh_handle.IndexBufferID);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(World_Chunk_CPUMeshVertex), reinterpret_cast<const void*>(offsetof(World_Chunk_CPUMeshVertex, X)));
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(World_Chunk_CPUMeshVertex), reinterpret_cast<const void*>(offsetof(World_Chunk_CPUMeshVertex, S)));
            glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(World_Chunk_CPUMeshVertex), reinterpret_cast<const void*>(offsetof(World_Chunk_CPUMeshVertex, F)));
            glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, sizeof(World_Chunk_CPUMeshVertex), reinterpret_cast<const void*>(offsetof(World_Chunk_CPUMeshVertex, L)));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glEnableVertexAttribArray(3);

            gpumesh_handle.IndicesCount = 0;

            // Move the cpu mesh out of chunk.
            auto cpumesh = std::move(chunk->CPUMesh);

            // Upload cpu mesh to gpu.
            glBufferData(GL_ARRAY_BUFFER, cpumesh.Vertices.size() * sizeof(World_Chunk_CPUMeshVertex), cpumesh.Vertices.data(), GL_STATIC_DRAW);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, cpumesh.Indices.size() * sizeof(std::uint32_t), cpumesh.Indices.data(), GL_STATIC_DRAW);

            gpumesh_handle.IndicesCount = static_cast<std::uint32_t>(cpumesh.Indices.size());

            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            auto res = GPUMeshHandleMap.emplace(chunk_id, std::move(gpumesh_handle));

            GPUMeshIDsToRender.push_back(chunk_id);
        }
        else
        {
            if (auto iter = GPUMeshHandleMap.find(chunk_id); iter != GPUMeshHandleMap.end())
            {
                GPUMeshIDsToRender.push_back(chunk_id);
            }
        }
    }

    void LoadShaderProgram()
    {
        // Load shader sources
        auto vertex_shader_path = "./resource/shader/Chunk.vert.glsl";
        auto fragment_shader_path = "./resource/shader/Chunk.frag.glsl";

        auto vertex_shader_source_opt = IO_ReadFile(vertex_shader_path);
        if (vertex_shader_source_opt.has_value() == false)
        {
            std::println("Failed to read {}", vertex_shader_path);
            return;
        }
        const char* vertex_shader_source = vertex_shader_source_opt.value().c_str();

        auto fragment_shader_source_opt = IO_ReadFile(fragment_shader_path);
        if (fragment_shader_source_opt.has_value() == false)
        {
            std::println("Failed to read {}", fragment_shader_path);
            return;
        }
        const char* fragment_shader_source = fragment_shader_source_opt.value().c_str();

        // Create vertex shader
        GLint compile_status;
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
        glCompileShader(vertex_shader);
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
        if (compile_status == GL_FALSE)
        {
            char info_log[512];
            glGetShaderInfoLog(vertex_shader, sizeof(info_log), nullptr, info_log);
            std::println("Error: Vertex Shader: {}", info_log);
            return;
        }

        // Create fragment shader
        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
        glCompileShader(fragment_shader);
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_status);
        if (compile_status == GL_FALSE)
        {
            char info_log[512];
            glGetShaderInfoLog(fragment_shader, sizeof(info_log), nullptr, info_log);
            std::println("Error: Fragment Shader: {}", info_log);
            return;
        }

        // Create shader program
        GLint link_status;
        GLuint program = glCreateProgram();
        glAttachShader(program, vertex_shader);
        glAttachShader(program, fragment_shader);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &link_status);
        if (link_status == GL_FALSE)
        {
            char info_log[512];
            glGetProgramInfoLog(program, sizeof(info_log), nullptr, info_log);
            std::println("Error: Shader Program: {}", info_log);
            return;
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        ChunkShaderProgram = program;
    }

    void LoadTexture()
    {
        auto image_opt = IO_ReadImage("./resource/texture/Blocks.png", true);

        if (image_opt.has_value() == false)
        {
            std::println("Failed to load ./resource/texture/Blocks.png");
            return;
        }

        auto& image = image_opt.value();

        GLint format = (image.ChannelNumbers == 4) ? GL_RGBA : GL_RGB;

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, format, image.Width, image.Height, 0, format, GL_UNSIGNED_BYTE, image.ImageData.data());
        //glGenerateMipmap(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, 0);

        BlocksTexture = texture;
    }
} // !namespace internal
