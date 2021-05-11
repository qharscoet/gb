cmake_minimum_required(VERSION 3.16)

# set the project name and version
project(gb VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_GENERATOR_PLATFORM x64)

file(GLOB SOURCES "./*.cpp")
set(WASM_LIB emu_wasm_lib.js)

# add_subdirectory (hqx)
add_subdirectory(../lib ./lib)

# add_library(gb_lib SHARED ${LIB_SOURCES})
# target_compile_definitions(gb_lib PRIVATE EMULATOR_EXPORTS)

add_executable(gb_wasm ${SOURCES})

target_include_directories(gb_wasm PUBLIC "../lib")
target_link_libraries(gb_wasm gb_lib)
em_link_js_library(gb_wasm ${WASM_LIB})
target_link_options(gb_wasm PRIVATE --bind)

target_link_options(gb_wasm PRIVATE --preload-file zelda.gbc)
target_link_options(gb_wasm PRIVATE -pthread)
# target_link_options(gb_wasm PRIVATE -s)
# target_link_options(gb_wasm PRIVATE --js-library ${WASM_LIB})