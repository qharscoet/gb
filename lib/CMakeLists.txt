cmake_minimum_required(VERSION 3.16)

# set the project name and version
project(libgb VERSION 1.0)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_GENERATOR_PLATFORM x64)

file(GLOB LIB_SOURCES "./*.cpp")

add_library(libgb SHARED ${LIB_SOURCES})
target_compile_definitions(libgb PRIVATE EMULATOR_EXPORTS)
target_compile_options(libgb PRIVATE -pthread -O3)
