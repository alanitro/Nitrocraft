#include "Nitrocraft.hpp"

#include <optional>
#include <array>
#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <format>
#include <sstream>
#include <iostream>
#include <fstream>
#include <print>
#include <glad/gl.h>
#include <glfw/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

namespace
{
GLFWwindow* g_Window = nullptr;

bool g_IsRunning = true;

std::vector<float> g_VertexData;
std::vector<float> g_IndexData;

// 24 vertices: 6 faces * 4 verts/face
// layout: x y z r g b
constexpr float g_CubeVertices[] = {
    // +Z (front) - Red
    -0.5f,-0.5f, 0.5f,  1.f,0.f,0.f,
     0.5f,-0.5f, 0.5f,  1.f,0.f,0.f,
     0.5f, 0.5f, 0.5f,  1.f,0.f,0.f,
    -0.5f, 0.5f, 0.5f,  1.f,0.f,0.f,

    // -Z (back) - Green
     0.5f,-0.5f,-0.5f,  0.f,1.f,0.f,
    -0.5f,-0.5f,-0.5f,  0.f,1.f,0.f,
    -0.5f, 0.5f,-0.5f,  0.f,1.f,0.f,
     0.5f, 0.5f,-0.5f,  0.f,1.f,0.f,

    // -X (left) - Blue
    -0.5f,-0.5f,-0.5f,  0.f,0.f,1.f,
    -0.5f,-0.5f, 0.5f,  0.f,0.f,1.f,
    -0.5f, 0.5f, 0.5f,  0.f,0.f,1.f,
    -0.5f, 0.5f,-0.5f,  0.f,0.f,1.f,

    // +X (right) - Yellow
     0.5f,-0.5f, 0.5f,  1.f,1.f,0.f,
     0.5f,-0.5f,-0.5f,  1.f,1.f,0.f,
     0.5f, 0.5f,-0.5f,  1.f,1.f,0.f,
     0.5f, 0.5f, 0.5f,  1.f,1.f,0.f,

    // +Y (top) - Magenta
    -0.5f, 0.5f, 0.5f,  1.f,0.f,1.f,
     0.5f, 0.5f, 0.5f,  1.f,0.f,1.f,
     0.5f, 0.5f,-0.5f,  1.f,0.f,1.f,
    -0.5f, 0.5f,-0.5f,  1.f,0.f,1.f,

    // -Y (bottom) - Cyan
    -0.5f,-0.5f,-0.5f,  0.f,1.f,1.f,
     0.5f,-0.5f,-0.5f,  0.f,1.f,1.f,
     0.5f,-0.5f, 0.5f,  0.f,1.f,1.f,
    -0.5f,-0.5f, 0.5f,  0.f,1.f,1.f,
};

constexpr unsigned int g_CubeIndices[] = {
    0,1,2,  0,2,3,        // front
    4,5,6,  4,6,7,        // back
    8,9,10, 8,10,11,      // left
    12,13,14, 12,14,15,   // right
    16,17,18, 16,18,19,   // top
    20,21,22, 20,22,23    // bottom
};

constexpr std::array<std::array<float, 12>, 6> g_BlockFaces
{
    std::array<float, 12>
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
    },
    std::array<float, 12>
    {
        1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 1.0f,
    },
    std::array<float, 12>
    {
        0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
    },
    std::array<float, 12>
    {
        0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
    },
    std::array<float, 12>
    {
        1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
    },
    std::array<float, 12>
    {
        0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
    },
};

std::optional<std::string> LoadFile(std::string_view filepath)
{
    std::ifstream file;
    file.open(std::string(filepath));

    if (file.is_open() == false)
    {
        std::println("Error: File {} does not exist", filepath);
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

std::optional<GLuint> LoadShaderProgram(std::string_view shader_name)
{
    std::string vertex_shader_path = std::format("{}{}{}", "./resource/shader/", shader_name, ".vert.glsl");
    std::string fragment_shader_path = std::format("{}{}{}", "./resource/shader/", shader_name, ".frag.glsl");
    
    // Create vertex shader
    auto vertex_shader_source_opt = LoadFile(vertex_shader_path);
    if (vertex_shader_source_opt.has_value() == false) return std::nullopt;
    const char* vertex_shader_source = vertex_shader_source_opt.value().c_str();

    GLint compile_status;
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compile_status);
    if (compile_status == GL_FALSE)
    {
        char info_log[512];
        glGetShaderInfoLog(vertex_shader, sizeof(info_log), nullptr, info_log);
        std::println(std::cerr, "ERROR:SHADER:VERTEX: {}", info_log);
        return std::nullopt;
    }

    // Create fragment shader
    auto fragment_shader_source_opt = LoadFile(fragment_shader_path);
    if (fragment_shader_source_opt.has_value() == false) return std::nullopt;
    const char* fragment_shader_source = fragment_shader_source_opt.value().c_str();

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compile_status);
    if (compile_status == GL_FALSE)
    {
        char info_log[512];
        glGetShaderInfoLog(fragment_shader, sizeof(info_log), nullptr, info_log);
        std::println(std::cerr, "ERROR:SHADER:FRAGMENT: {}", info_log);
        return std::nullopt;
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
        glGetProgramInfoLog(fragment_shader, sizeof(info_log), nullptr, info_log);
        std::println(std::cerr, "ERROR: SHADER PROGRAM: {}", info_log);
        return std::nullopt;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}
} // namespace unnamed

void Nitrocraft::Run()
{
    // Initialize
    glfwSetErrorCallback(
        [](int error_code, const char* description)
        {
            (void)error_code;
            std::println(std::cerr, "Error: GLFW: {}", description);
        }
    );

    if (glfwInit() == GLFW_FALSE)
    {
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    g_Window = glfwCreateWindow(1720, 960, "Nitrocraft", nullptr, nullptr);
    if (!g_Window)
    {
        glfwTerminate();
        return;
    }

    glfwFocusWindow(g_Window);

    glfwMakeContextCurrent(g_Window);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        glfwDestroyWindow(g_Window);
        glfwTerminate();
        return;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    std::println("GL Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    std::println("GL Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    std::println("GL Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    glfwSetFramebufferSizeCallback(
        g_Window,
        [](GLFWwindow* window, int width, int height)
        {
            (void)window;
            glViewport(0, 0, width, height);
        }
    );

    //// Create vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //// Create vertex buffer
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_CubeVertices), g_CubeVertices, GL_STATIC_DRAW);

    //// Create index buffer
    GLuint ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(g_CubeIndices), g_CubeIndices, GL_STATIC_DRAW);

    //// Specify vertex attrib
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, reinterpret_cast<const void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, reinterpret_cast<const void*>(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    GLenum error =glGetError();
    if (error != GL_NO_ERROR)
    {
        std::println("ERROR: OpenGL");
    }

    //// Load Program
    auto program_opt = LoadShaderProgram("Cube");
    if (program_opt.has_value() == false) return;
    GLuint program = program_opt.value();


    //// Configs
    glUseProgram(program);

    glEnable(GL_DEPTH_TEST);

    // Loop
    while (g_IsRunning)
    {
        if (glfwWindowShouldClose(g_Window))
        {
            g_IsRunning = false;
        }

        if (glfwGetKey(g_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            g_IsRunning = false;
        }

        //// Create transform matrix
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -2.0f));
        model = glm::rotate(model, static_cast<float>(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1720.0f / 960.0f, 0.1f, 100.0f);
        glm::mat4 model_view_projection = projection * model;

        GLint mvp_location = glGetUniformLocation(program, "u_ModelViewProjection");
        if (mvp_location == -1)
        {
            std::println("Uniform location for u_ModelViewProjection not found");
        }
        
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(model_view_projection));

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glDrawElements(GL_TRIANGLES, sizeof(g_CubeIndices) / sizeof(g_CubeIndices[0]), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(g_Window);

        glfwPollEvents();
    }

    // Terminate
    glfwDestroyWindow(g_Window);

    glfwTerminate();
}
