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
#include "World_Mesh.hpp"
#include "Utility_Time.hpp"

namespace
{
    struct ChunkGPUMesh
    {
        GLuint VertexArrayID;
        GLuint VertexBufferID;
        GLuint IndexBufferID;
        std::uint32_t IndicesCount;
    };

    GLuint ChunkShaderProgram = 0;
    GLuint BlocksTexture = 0;

    std::vector<ChunkGPUMesh*>                      ChunksToRender;
    std::unordered_map<World_ChunkID, ChunkGPUMesh> ChunkMeshes;


    void PushChunksToRender(const World_Chunk* chunk);
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
    for (auto& [chunk_id, chunk_mesh] : ChunkMeshes)
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

    for (auto chunk_mesh : ChunksToRender)
    {
        glBindVertexArray(chunk_mesh->VertexArrayID);

        glDrawElements(GL_TRIANGLES, chunk_mesh->IndicesCount, GL_UNSIGNED_INT, reinterpret_cast<const void*>(0));
    }

    ChunksToRender.clear();
}

void WorldRenderer_PrepareChunksToRender(const World_ActiveArea& active_area)
{
    for (int ix = 3; ix < World_LOADING_DIAMETER - 3; ix++)
    {
        for (int iz = 3; iz < World_LOADING_DIAMETER - 3; iz++)
        {
            PushChunksToRender(active_area.At(ix, iz));
        }
    }
}

namespace // internal
{
    void PushChunksToRender(const World_Chunk* chunk)
    {
        World_ChunkID chunk_id = chunk->ID;
        World_GlobalXYZ chunk_offset = World_FromChunkIDToChunkOffset(chunk->ID);

        if (auto iter = ChunkMeshes.find(chunk_id); iter != ChunkMeshes.end())
        {
            ChunksToRender.push_back(&iter->second);
            return;
        }

        // Create temp cpu mesh
        static std::vector<World_Mesh_ChunkCPUMeshVertex> vertices(World_CHUNK_VOLUME * 6 * 4);
        static std::vector<std::uint32_t>   indices(World_CHUNK_VOLUME * 6 * 6);
        vertices.clear();
        indices.clear();

        World_Mesh_GenerateChunkCPUMesh(chunk, vertices, indices);

        // Create gpu mesh and upload vertices and indices data
        ChunkGPUMesh chunk_gpu_mesh;
        glGenVertexArrays(1, &chunk_gpu_mesh.VertexArrayID);
        glGenBuffers(1, &chunk_gpu_mesh.VertexBufferID);
        glGenBuffers(1, &chunk_gpu_mesh.IndexBufferID);

        glBindVertexArray(chunk_gpu_mesh.VertexArrayID);
        glBindBuffer(GL_ARRAY_BUFFER, chunk_gpu_mesh.VertexBufferID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk_gpu_mesh.IndexBufferID);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(World_Mesh_ChunkCPUMeshVertex), reinterpret_cast<const void*>(offsetof(World_Mesh_ChunkCPUMeshVertex, X)));
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(World_Mesh_ChunkCPUMeshVertex), reinterpret_cast<const void*>(offsetof(World_Mesh_ChunkCPUMeshVertex, S)));
        glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, sizeof(World_Mesh_ChunkCPUMeshVertex), reinterpret_cast<const void*>(offsetof(World_Mesh_ChunkCPUMeshVertex, F)));
        glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, sizeof(World_Mesh_ChunkCPUMeshVertex), reinterpret_cast<const void*>(offsetof(World_Mesh_ChunkCPUMeshVertex, L)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);

        chunk_gpu_mesh.IndicesCount = 0;

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(World_Mesh_ChunkCPUMeshVertex), vertices.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(std::uint32_t), indices.data(), GL_STATIC_DRAW);

        chunk_gpu_mesh.IndicesCount = static_cast<std::uint32_t>(indices.size());

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        auto res = ChunkMeshes.emplace(chunk_id, chunk_gpu_mesh);

        ChunksToRender.push_back(&res.first->second);
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
