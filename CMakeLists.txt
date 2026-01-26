cmake_minimum_required(VERSION 3.20)

project(Nitrocraft)

add_executable(${PROJECT_NAME}
    source/Main.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
)
