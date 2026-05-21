#include "Nitrocraft.hpp"

#include <print>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "World.hpp"
#include "Graphics_BlockOutlineRenderer.hpp"
#include "Graphics_ChunkOutlineRenderer.hpp"
#include "Graphics_WorldRenderer.hpp"
#include "Graphics_Camera.hpp"
#include "Utility_Time.hpp"
#include "Utility_Timer.hpp"

namespace nitrocraft
{

namespace
{

enum NitrocraftState
{
    INACTIVE,
    ACTIVE,
    PAUSE,
};

NitrocraftState s_state = NitrocraftState::INACTIVE;

bool s_render_block_outline = true;

bool s_render_chunk_outline = false;

float s_player_speed = 40.0f;

int s_render_distance = 10;

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
                s_state = flip ? NitrocraftState::PAUSE : NitrocraftState::ACTIVE;
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

    glm::vec3 delta_position(0.0f);

    if (s_state == NitrocraftState::ACTIVE)
    {
        //// Get delta position
        float speed = s_player_speed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)    speed *= 2.0f;
        float delta_speed = speed * static_cast<float>(delta_time);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)             delta_position += camera.GetLeft() * delta_speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)             delta_position += camera.GetRight() * delta_speed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)         delta_position += glm::vec3(0.0f, 1.0f, 0.0f) * delta_speed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)  delta_position += glm::vec3(0.0f, -1.0f, 0.0f) * delta_speed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)             delta_position += camera.GetBack() * delta_speed;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)             delta_position += camera.GetFront() * delta_speed;
    }

    //// Get delta rotation
    glm::vec2 delta_rotation(0.0f);
    static double prev_xpos, prev_ypos;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    if (s_state == NitrocraftState::ACTIVE)
    {
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
    }
    else
    {
        prev_xpos = xpos;
        prev_ypos = ypos;
    }

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

void ImGUI_Update(
    GLFWwindow* window,
    world::World& world,
    graphics::WorldRenderer& world_renderer,
    const Camera& camera,
    const std::optional<world::RayResult>& raycast_result_opt
)
{
    if (ImGui::Begin("Information & Configs") == false)
    {
        ImGui::End();

        return;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    auto io = ImGui::GetIO();
    ImGui::Text("Frame Rate : %d FPS", (int)io.Framerate);
    ImGui::Text("Frame Time : %.3f ms/frame", 1000.0f / io.Framerate);
    ImGui::Text("Resolution : %d x %d", width, height);
    ImGui::Text(" ");

    constexpr const char* state_names[3]{ "Inactive", "Active", "Pause" };
    ImGui::Text("State : %s", state_names[(int)s_state]);
    ImGui::Text(" ");

    ImGui::Text("X Position : %.2f", camera.GetPosition().x);
    ImGui::Text("Y Position : %.2f", camera.GetPosition().y);
    ImGui::Text("Z Position : %.2f", camera.GetPosition().z);
    ImGui::Text(" ");

    ImGui::Text("X Rotation : %.2f", camera.GetFront().x);
    ImGui::Text("Y Rotation : %.2f", camera.GetFront().y);
    ImGui::Text("Z Rotation : %.2f", camera.GetFront().z);
    ImGui::Text(" ");

    ImGui::Text("Chunk Gen Threads: %d", world.GetChunkManager().GetWorkerThreadCount());
    ImGui::Text(" ");

    ImGui::Text("Chunks Loaded: %d", world.GetChunkManager().GetLoadedChunkCount());
    ImGui::Text(" ");

    auto id = world::FromGlobalToChunkID(camera.GetPosition());
    ImGui::Text("Current Chunk ID : %d %d", id.x, id.z);
    ImGui::Text(" ");

    auto chunk = world.GetChunkAt(camera.GetPosition());
    ImGui::Text("Current Chunk Stage : %d", chunk->stage.load(std::memory_order_relaxed));
    ImGui::Text(" ");

    ImGui::Text("Sunlight Level : %02d", (int)world::ExtractSunlight(world.GetLightAt(camera.GetPosition())));
    ImGui::Text("Pointlight Level : %02d", (int)world::ExtractPointlight(world.GetLightAt(camera.GetPosition())));
    ImGui::Text(" ");

    ImGui::Text(
        "Selected Block: %s",
        raycast_result_opt.has_value() ? std::string(world.GetBlockAt(raycast_result_opt.value().position).GetBlockName()).c_str() : "None"
    );
    ImGui::Text(
        "Selected Face: %s",
        raycast_result_opt.has_value() ? "XN\0XP\0YN\0YP\0ZN\0ZP" + (std::intptr_t)raycast_result_opt.value().face * 3 : "None"
    );
    ImGui::Text(" ");

    ImGui::Text("Movement Speed:");
    ImGui::SliderFloat("##a", &s_player_speed, 1.0f, 100.0f);

    ImGui::Text("Render Distance:");
    ImGui::SliderInt("##b", &s_render_distance, 2, 32);
    world.SetRenderDistance(s_render_distance);

    ImGui::Text("Ambient Occlusion:");
    static bool enable_ambient_occlusion = true;
    ImGui::Checkbox("##c", &enable_ambient_occlusion);
    world_renderer.EnableAmbientOcclusion(enable_ambient_occlusion);

    ImGui::Text("Block Outline:");
    ImGui::Checkbox("##d", &s_render_block_outline);

    ImGui::Text("Chunk Outline:");
    ImGui::Checkbox("##e", &s_render_chunk_outline);

    ImGui::Text("Wireframe Mode:");
    static bool line_mode = false;
    ImGui::Checkbox("##f", &line_mode);
    if (line_mode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    ImGui::End();
}

void ImGUI_Render()
{
    // Rendering
    // (Your code clears your framebuffer, renders your other stuff etc.)
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    // (Your code calls glfwSwapBuffers() etc.)
}

} // namespace

void Run()
{
    // Initialization
    GLFWwindow* window = InitializeGLFWAndOpenGLContext();

    ImGUI_Initialize(window);

    world::World world;

    graphics::WorldRenderer world_renderer;

    graphics::BlockOutlineRenderer block_outline_renderer;

    graphics::ChunkOutlineRenderer chunk_outline_renderer;

    world.Initialize();

    world_renderer.Initialize();

    block_outline_renderer.Initialize();

    chunk_outline_renderer.Initialize();

    bool is_running = true;

    s_state = NitrocraftState::ACTIVE;

    utility::Timer timer;

    Camera camera;
    camera.SetAspectRatio(1720.0f / 960.0f);
    camera.SetFar(640.0f);
    camera.Calculate(glm::vec3(0.0f, 128.0f, 0.0f), glm::vec3(0.0f));

    //// Pipeline config
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Loop
    while (is_running)
    {
        //// Update
        ImGUI_NewFrame();

        if (glfwWindowShouldClose(window))
        {
            is_running = false;
        }

        RecalculateCamera(camera, window, timer.Elapsed());

        timer.Reset();

        world.Update(camera);

        world_renderer.PrepareChunksToRender(world.GetChunkManager().GetChunksInRenderArea_MainThread());

        auto raycast_result_opt = world.CastRay(camera.GetPosition(), camera.GetFront(), 10.0f);

        ImGUI_Update(window, world, world_renderer, camera, raycast_result_opt);

        //// Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        world_renderer.Render(camera, world.GetSunlightIntensity(), world.GetSkyColor());

        if (s_render_block_outline && raycast_result_opt.has_value())
        {
            block_outline_renderer.Render(camera.GetProjection(), raycast_result_opt.value().position);
        }

        if (s_render_chunk_outline)
        {
            chunk_outline_renderer.Render(camera.GetViewProjection(), world::FromGlobalToChunkOffset(camera.GetPosition()));
        }

        ImGUI_Render();

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    // Terminate

    chunk_outline_renderer.Terminate();

    block_outline_renderer.Terminate();

    world_renderer.Terminate();

    world.Terminate();

    ImGUI_Terminate();
    
    glfwDestroyWindow(window);

    glfwTerminate();
}

} // namespace nitrocraft
