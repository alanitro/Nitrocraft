cmake_minimum_required(VERSION 3.20)

project(Nitrocraft)

# Executable
add_executable(${PROJECT_NAME}
    source/Main.cpp
    source/Nitrocraft.hpp
    source/Nitrocraft.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
)

# Libraries
add_subdirectory(vendor/glad)
add_subdirectory(vendor/glfw)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(${PROJECT_NAME} PRIVATE
    glad
    glfw
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/resource
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/resource
)

