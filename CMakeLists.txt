cmake_minimum_required(VERSION 3.16)

# set the project name and version
project(gb VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_GENERATOR_PLATFORM x64)

file(GLOB LIB_SOURCES "./lib/*.cpp")
file(GLOB SOURCES "./src/*.cpp")
file(GLOB IMGUI "./src/imgui/*.cpp")
file(GLOB HQX_S "./src/hqx/*.c")

# add_subdirectory (hqx)

add_library(gb_lib SHARED ${LIB_SOURCES})
target_compile_definitions(gb_lib PRIVATE EMULATOR_EXPORTS)

add_executable(gb ${SOURCES} ${IMGUI} ${HQX_S} "./src/tinyfiledialogs/tinyfiledialogs.c")

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)


if(DEFINED SDL2_INCLUDE_DIRS)
    target_include_directories(gb PRIVATE ${SDL2_INCLUDE_DIRS})
endif()

# need to properly filter out if vcpkg or not
if (UNIX)
    target_link_libraries(gb PRIVATE SDL2)
    target_link_libraries(gb PRIVATE pthread)
else()
    target_link_libraries(gb PRIVATE SDL2::SDL2 )
endif()

target_include_directories(gb PRIVATE "./lib")
target_link_libraries(gb PRIVATE gb_lib OpenGL::GL GLEW::GLEW)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(MSVC)
        target_link_options(gb PUBLIC /entry:mainCRTStartup /subsystem:windows)
    endif(MSVC)
endif(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")