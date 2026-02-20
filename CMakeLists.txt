cmake_minimum_required(VERSION 3.20)

project(Nitrocraft)

# Executable
add_executable(${PROJECT_NAME}
    source/Main.cpp

    source/Nitrocraft.hpp
    source/Nitrocraft.cpp

    source/World.hpp
    source/World.cpp
    source/World_Coordinate.hpp
    source/World_Block.hpp
    source/World_Block.cpp
    source/World_Chunk.hpp
    source/World_Chunk.cpp
    source/World_ChunkManager.hpp
    source/World_ChunkManager.cpp
    source/World_Generation.hpp
    source/World_Generation.cpp
    source/World_Light.hpp
    source/World_Light.cpp

    source/Graphics_Camera.hpp
    source/Graphics_Camera.cpp
    source/Graphics_Shader.hpp
    source/Graphics_Shader.cpp
    source/Graphics_Mesh.hpp
    source/Graphics_Mesh.cpp
    source/Graphics_WorldRenderer.hpp
    source/Graphics_WorldRenderer.cpp
    source/Graphics_BlockOutlineRenderer.hpp
    source/Graphics_BlockOutlineRenderer.cpp

    source/Utility_Time.hpp
    source/Utility_Timer.hpp
    source/Utility_Array2D.hpp
    source/Utility_Array3D.hpp
    source/Utility_BlockingQueue.hpp
    source/Utility_IO.hpp
    source/Utility_IO.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
)

# Libraries
add_subdirectory(vendor/glad)
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glm)
add_subdirectory(vendor/stb)
add_subdirectory(vendor/FastNoise2)
add_subdirectory(vendor/imgui)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
    glm
    stb
    FastNoise2
    imgui
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/resource
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/resource
)
