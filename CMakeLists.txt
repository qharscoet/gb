cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(gb VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_GENERATOR_PLATFORM x86)

file(GLOB SOURCES "./src/*.cpp")
file(GLOB IMGUI "./src/imgui/*.cpp")

add_executable(gb ${SOURCES} ${IMGUI} "./src/tinyfiledialogs/tinyfiledialogs.c")

find_package(SDL2 CONFIG REQUIRED)
find_package(GLEW REQUIRED)

target_link_libraries(gb PRIVATE SDL2::SDL2 SDL2::SDL2main GLEW::GLEW)