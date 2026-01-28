cmake_minimum_required(VERSION 3.20)

add_library(stb STATIC ./stb/stb_image.c)

target_include_directories(stb PUBLIC .)
