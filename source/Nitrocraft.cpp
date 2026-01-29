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
#include "Timer.hpp"
#include "ResourceManager.hpp"
#include "Camera.hpp"

namespace
{
GLFWwindow* g_Window = nullptr;

bool g_IsRunning = true;

constexpr float tileSize = 1.0f / 16.0f;
constexpr int tx = 1;
constexpr int ty = 0;
// bottom-left origin UVs:
constexpr float s0 = tx * tileSize;          // 0/16
constexpr float t0 = ty * tileSize;          // 1/16
constexpr float s1 = (tx + 1) * tileSize;    // 1/16
constexpr float t1 = (ty + 1) * tileSize;    // 2/16
constexpr float g_CubeVertices[] = {
    // +Z (front)
    -0.5f,-0.5f, 0.5f,  s0,t0,
     0.5f,-0.5f, 0.5f,  s1,t0,
     0.5f, 0.5f, 0.5f,  s1,t1,
    -0.5f, 0.5f, 0.5f,  s0,t1,

    // -Z (back)
     0.5f,-0.5f,-0.5f,  s0,t0,
    -0.5f,-0.5f,-0.5f,  s1,t0,
    -0.5f, 0.5f,-0.5f,  s1,t1,
     0.5f, 0.5f,-0.5f,  s0,t1,

     // -X (left)
     -0.5f,-0.5f,-0.5f,  s0,t0,
     -0.5f,-0.5f, 0.5f,  s1,t0,
     -0.5f, 0.5f, 0.5f,  s1,t1,
     -0.5f, 0.5f,-0.5f,  s0,t1,

     // +X (right)
      0.5f,-0.5f, 0.5f,  s0,t0,
      0.5f,-0.5f,-0.5f,  s1,t0,
      0.5f, 0.5f,-0.5f,  s1,t1,
      0.5f, 0.5f, 0.5f,  s0,t1,

      // +Y (top)
      -0.5f, 0.5f, 0.5f,  s0,t0,
       0.5f, 0.5f, 0.5f,  s1,t0,
       0.5f, 0.5f,-0.5f,  s1,t1,
      -0.5f, 0.5f,-0.5f,  s0,t1,

      // -Y (bottom)
      -0.5f,-0.5f,-0.5f,  s0,t0,
       0.5f,-0.5f,-0.5f,  s1,t0,
       0.5f,-0.5f, 0.5f,  s1,t1,
      -0.5f,-0.5f, 0.5f,  s0,t1,
};


constexpr unsigned int g_CubeIndices[] = {
    0,1,2,  0,2,3,        // front
    4,5,6,  4,6,7,        // back
    8,9,10, 8,10,11,      // left
    12,13,14, 12,14,15,   // right
    16,17,18, 16,18,19,   // top
    20,21,22, 20,22,23    // bottom
};

constexpr glm::vec3 g_CubePositions[] =
{
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};
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

    //// Create window
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

    //// Load OpenGL functions
    glfwMakeContextCurrent(g_Window);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        glfwDestroyWindow(g_Window);
        glfwTerminate();
        return;
    }

    std::println("GL Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    std::println("GL Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    std::println("GL Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    //// Register callbacks
    glfwSetFramebufferSizeCallback(
        g_Window,
        [](GLFWwindow* window, int width, int height)
        {
            (void)window;
            glViewport(0, 0, width, height);
        }
    );

    glfwSetKeyCallback(
        g_Window,
        [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            (void)window;
            (void)scancode;
            (void)mods;

            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                static bool flip = false;
                glfwSetInputMode(g_Window, GLFW_CURSOR, flip ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
                flip = flip ? false : true;
            }
        }
    );

    glfwSetInputMode(g_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(g_Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, reinterpret_cast<const void*>(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, reinterpret_cast<const void*>(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::println("ERROR: OpenGL");
    }

    //// Load shader program
    auto program_opt = ResourceManager::LoadShaderProgram("Block");
    if (program_opt.has_value() == false) return;
    GLuint program = program_opt.value();

    //// Load texture program
    auto texture_opt = ResourceManager::LoadTexture("Blocks");
    if (texture_opt.has_value() == false) return;
    GLuint texture = texture_opt.value();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    //// Graphics configs
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glUseProgram(program);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    GLint texture_location = glGetUniformLocation(program, "u_Texture");
    if (texture_location == -1)
    {
        std::println("Uniform location for u_Texture not found");
    }
    glUniform1i(texture_location, 0);

    //// Camera
    Camera camera;
    camera.SetAspectRatio(1720.0f / 960.0f);
    camera.SetFar(1000.0f);

    //// Timer
    Timer timer;

    //// Player info
    float player_speed = 3.0f;

    bool first_loop = true;

    // Loop
    while (g_IsRunning)
    {
        //// Update
        if (glfwWindowShouldClose(g_Window))
        {
            g_IsRunning = false;
        }

        //// Get delta position
        glm::vec3 delta_position{};
        float speed = player_speed;
        if (glfwGetKey(g_Window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)    speed *= 5.0f;
        float delta_speed = speed * static_cast<float>(timer.Elapsed());
        if (glfwGetKey(g_Window, GLFW_KEY_A) == GLFW_PRESS)             delta_position += camera.GetLeft() * delta_speed;
        if (glfwGetKey(g_Window, GLFW_KEY_D) == GLFW_PRESS)             delta_position += camera.GetRight() * delta_speed;
        if (glfwGetKey(g_Window, GLFW_KEY_SPACE) == GLFW_PRESS)         delta_position += camera.GetUp() * delta_speed;
        if (glfwGetKey(g_Window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)  delta_position += camera.GetDown() * delta_speed;
        if (glfwGetKey(g_Window, GLFW_KEY_S) == GLFW_PRESS)             delta_position += camera.GetBack() * delta_speed;
        if (glfwGetKey(g_Window, GLFW_KEY_W) == GLFW_PRESS)             delta_position += camera.GetFront() * delta_speed;
        timer.Reset();

        //// Get delta rotatin
        static double prev_xpos, prev_ypos;
        double xpos, ypos;
        glfwGetCursorPos(g_Window, &xpos, &ypos);
        glm::vec2 delta_rotation;
        if (first_loop)
        {
            delta_rotation = glm::vec2(0);
            first_loop = false;
        }
        else
        {
            float xoffset = static_cast<float>((xpos - prev_xpos) / 1000.0);
            float yoffset = static_cast<float>((ypos - prev_ypos) / 1000.0);
            delta_rotation = glm::vec2(xoffset, -yoffset);
        }
        prev_xpos = xpos;
        prev_ypos = ypos;

        //// Recalculate camera
        camera.Calculate(delta_position, delta_rotation);

        //// Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //// Create transform matrix
        GLint mvp_location = glGetUniformLocation(program, "u_ModelViewProjection");
        if (mvp_location == -1)
        {
            std::println("Uniform location for u_ModelViewProjection not found");
        }

        for (auto position : g_CubePositions)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, position);
            glm::mat4 model_view_projection = camera.GetViewProjection() * model;
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(model_view_projection));
            glDrawElements(GL_TRIANGLES, sizeof(g_CubeIndices) / sizeof(g_CubeIndices[0]), GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(g_Window);

        glfwPollEvents();
    }

    // Terminate
    glfwDestroyWindow(g_Window);

    glfwTerminate();
}
