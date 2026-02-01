#include "Nitrocraft.hpp"

#include <print>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include "World.hpp"
#include "Graphics_WorldRenderer.hpp"
#include "Graphics_Camera.hpp"
#include "Utility_Timer.hpp"

namespace
{
GLFWwindow* InitializeGLFWAndOpenGLContext()
{
    GLFWwindow* window = nullptr;
    // Initialize
    glfwSetErrorCallback(
        [](int error_code, const char* description)
        {
            (void)error_code;
            std::println("Error: GLFW: {}", description);
        }
    );

    if (glfwInit() == GLFW_FALSE)
    {
        return nullptr;
    }

    //// Create window
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(1720, 960, "Nitrocraft", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return nullptr;
    }

    glfwFocusWindow(window);

    //// Load OpenGL functions
    glfwMakeContextCurrent(window);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return nullptr;
    }

    std::println("GL Version: {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    std::println("GL Vendor: {}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
    std::println("GL Renderer: {}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));

    //// Register callbacks
    glfwSetFramebufferSizeCallback(
        window,
        [](GLFWwindow* window, int width, int height)
        {
            (void)window;
            glViewport(0, 0, width, height);
        }
    );

    glfwSetKeyCallback(
        window,
        [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            (void)window;
            (void)scancode;
            (void)mods;

            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                static bool flip = false;
                glfwSetInputMode(window, GLFW_CURSOR, flip ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
                flip = flip ? false : true;
            }
        }
    );

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glViewport(0, 0, 1720, 960);

    return window;
}

void RecalculateCamera(Camera& camera, GLFWwindow* window, double delta_time)
{
    static bool first_loop = true;

    float player_speed = 3.0f;

    //// Get delta position
    glm::vec3 delta_position{};
    float speed = player_speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)    speed *= 5.0f;
    float delta_speed = speed * static_cast<float>(delta_time);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)             delta_position += camera.GetLeft() * delta_speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)             delta_position += camera.GetRight() * delta_speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)         delta_position += glm::vec3(0.0f, 1.0f, 0.0f) * delta_speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)  delta_position += glm::vec3(0.0f, -1.0f, 0.0f) * delta_speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)             delta_position += camera.GetBack() * delta_speed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)             delta_position += camera.GetFront() * delta_speed;

    //// Get delta rotatin
    static double prev_xpos, prev_ypos;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
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
}
} // namespace unnamed

void Nitrocraft::Run()
{
    // Initialization
    GLFWwindow* window = InitializeGLFWAndOpenGLContext();

    bool is_running = true;

    Timer timer;

    Camera camera;
    camera.SetAspectRatio(1720.0f / 960.0f);
    camera.SetFar(1000.0f);
    camera.Calculate(glm::vec3(0.0f, 64.0f, 0.0f), glm::vec3(0.0f));

    World world;
    world.Initialize();

    WorldRenderer world_renderer;
    world_renderer.Initialize();

    //// Pipeline config
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //// Player info

    // Loop
    while (is_running)
    {
        //// Update
        if (glfwWindowShouldClose(window)) is_running = false;

        RecalculateCamera(camera, window, timer.Elapsed());

        timer.Reset();

        world_renderer.PushChunksToRender(ChunkID(0, 0, 0), world.GetChunkAt(ChunkID(0, 0, 0)));

        //// Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        world_renderer.Render(camera);

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    // Terminate
    world.Terminate();

    world_renderer.Terminate();
    
    glfwDestroyWindow(window);

    glfwTerminate();
}
