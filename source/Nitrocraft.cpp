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

namespace
{
GLFWwindow* g_Window = nullptr;

bool g_IsRunning = true;

std::vector<float> g_VertexData;
std::vector<float> g_IndexData;

float g_Vertices[]
{
    -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
     0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
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

    //// Create vertex buffer
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_Vertices), g_Vertices, GL_STATIC_DRAW);

    //// Create vertex array object
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, reinterpret_cast<const void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, reinterpret_cast<const void*>(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    //// Load Program
    auto program_opt = LoadShaderProgram("Triangle");
    if (program_opt.has_value() == false) return;
    GLuint program = program_opt.value();

    glUseProgram(program);

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

        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(g_Window);

        glfwPollEvents();
    }

    // Terminate
    glfwDestroyWindow(g_Window);

    glfwTerminate();
}
