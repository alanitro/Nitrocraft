#include "Nitrocraft.hpp"

#include <print>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "World.hpp"
#include "Graphics_BlockOutlineRenderer.hpp"
#include "Graphics_WorldRenderer.hpp"
#include "Graphics_Camera.hpp"
#include "Utility_Time.hpp"
#include "Utility_Timer.hpp"

namespace
{
enum State
{
    INACTIVE,
    ACTIVE,
    PAUSE,
};

float PlayerSpeed = 10.0f;

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

    //// Get delta position
    glm::vec3 delta_position{};
    float speed = PlayerSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)    speed *= 5.0f;
    float delta_speed = speed * static_cast<float>(delta_time);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)             delta_position += camera.GetLeft() * delta_speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)             delta_position += camera.GetRight() * delta_speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)         delta_position += glm::vec3(0.0f, 1.0f, 0.0f) * delta_speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)  delta_position += glm::vec3(0.0f, -1.0f, 0.0f) * delta_speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)             delta_position += camera.GetBack() * delta_speed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)             delta_position += camera.GetFront() * delta_speed;

    //// Get delta rotation
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

void ImGUI_Initialize(GLFWwindow* window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.FontGlobalScale = 1.6f;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    ImGui::StyleColorsDark();
}

void ImGUI_Terminate()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGUI_NewFrame()
{
    // (Your code calls glfwPollEvents())
    // ...
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGUI_Render()
{
    // Rendering
    // (Your code clears your framebuffer, renders your other stuff etc.)
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // (Your code calls glfwSwapBuffers() etc.)
}
} // namespace unnamed

void Nitrocraft_Run()
{
    // Initialization
    GLFWwindow* window = InitializeGLFWAndOpenGLContext();

    ImGUI_Initialize(window);

    bool is_running = true;

    State state = State::ACTIVE;

    Timer timer;

    Camera camera;
    camera.SetAspectRatio(1720.0f / 960.0f);
    camera.SetFar(640.0f);
    camera.Calculate(glm::vec3(0.0f, 96.0f, 0.0f), glm::vec3(0.0f));

    Graphics_BlockOutlineRenderer_Initialize();

    Graphics_WorldRenderer_Initialize();

    World_Initialize();

    //// Pipeline config
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    //// Player info

    // Loop
    while (is_running)
    {
        //// Update
        ImGUI_NewFrame();

        if (glfwWindowShouldClose(window)) is_running = false;

        if (state == State::ACTIVE) RecalculateCamera(camera, window, timer.Elapsed());

        timer.Reset();

        World_Update(camera);

        WorldRenderer_PrepareChunksToRender(World_GetChunkManager().GetChunksInRenderArea());

        auto raycast_result_opt = World_CastRay(camera.GetPosition(), camera.GetFront(), 10.0f);

        if (ImGui::Begin("Information & Configs"))
        {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            auto io = ImGui::GetIO();
            ImGui::Text("Frame Rate : %d FPS", (int)io.Framerate);
            ImGui::Text("Frame Time : %.3f ms/frame", 1000.0f / io.Framerate);
            ImGui::Text("Resolution : %d x %d", width, height);
            ImGui::Text(" ");

            constexpr const char* state_names[3]{ "Inactive", "Active", "Pause" };
            ImGui::Text("State : %s", state_names[(int)state]);
            ImGui::Text(" ");

            ImGui::Text("X Position : %.2f", camera.GetPosition().x);
            ImGui::Text("Y Position : %.2f", camera.GetPosition().y);
            ImGui::Text("Z Position : %.2f", camera.GetPosition().z);
            ImGui::Text(" ");

            ImGui::Text("X Rotation : %.2f", camera.GetFront().x);
            ImGui::Text("Y Rotation : %.2f", camera.GetFront().y);
            ImGui::Text("Z Rotation : %.2f", camera.GetFront().z);
            ImGui::Text(" ");

            ImGui::Text("Chunk Gen Threads: %d", World_GetChunkManager().GetWorkerThreadCount());
            ImGui::Text(" ");

            ImGui::Text("Chunks Loaded: %d", World_GetChunkManager().GetLoadedChunkCount());
            ImGui::Text(" ");

            auto id = World_FromGlobalToChunkID(camera.GetPosition());
            ImGui::Text("Current Chunk ID : %d %d", id.x, id.z);
            ImGui::Text(" ");

            auto chunk = World_GetChunkAt(camera.GetPosition());
            ImGui::Text("Current Chunk Stage : %d", chunk->Stage.load(std::memory_order_relaxed));
            ImGui::Text(" ");

            ImGui::Text("Sunlight Level : %02d", (int)World_ExtractSunlight(World_GetLightAt(camera.GetPosition())));
            ImGui::Text("Pointlight Level : %02d", (int)World_ExtractPointlight(World_GetLightAt(camera.GetPosition())));
            ImGui::Text(" ");

            ImGui::Text(
                "Selected Block: %s",
                raycast_result_opt.has_value() ? std::string(World_GetBlockAt(raycast_result_opt.value().first).GetBlockName()).c_str() : "None"
            );
            ImGui::Text(
                "Selected Face: %s",
                raycast_result_opt.has_value() ? "XN\0XP\0YN\0YP\0ZN\0ZP" + (int)raycast_result_opt.value().second * 3 : "None"
            );
            ImGui::Text(" ");

            static bool line_mode = false;
            ImGui::Checkbox(" Line Mode", &line_mode);
            if (line_mode)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            ImGui::Text(" ");

            ImGui::SliderFloat(" Speed", &PlayerSpeed, 1.0f, 100.0f);
            ImGui::Text(" ");
        }
        ImGui::End();

        //// Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        WorldRenderer_Render(camera, World_GetSunlightIntensity(), World_GetSkyColor());

        if (raycast_result_opt.has_value())
        {
            World_Position position = raycast_result_opt.value().first;

            Graphics_BlockOutlineRenderer_RenderBlock(camera, position);
        }

        ImGUI_Render();

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    // Terminate
    World_Terminate();

    WorldRenderer_Terminate();

    ImGUI_Terminate();
    
    glfwDestroyWindow(window);

    glfwTerminate();
}
