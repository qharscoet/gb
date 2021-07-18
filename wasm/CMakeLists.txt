cmake_minimum_required(VERSION 3.16)

# set the project name and version
project(gb_wasm VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_GENERATOR_PLATFORM x64)

file(GLOB SOURCES "./*.cpp")
set(WASM_LIB emu_wasm_lib.js)

option(USE_SDL "Use SDL wasm ports" OFF)

if(USE_SDL)
	set(SDL_SRC "../src/sdl_audio.cpp" "../src/sdl_display.cpp")
else()
	set(SDL_SRC "../src/sdl_audio.cpp")
endif()


add_subdirectory(../lib ./libgb)
add_subdirectory(../hqx ./hqx)

add_executable(gb_wasm ${SOURCES} ${SDL_SRC})

if(USE_SDL)
	target_compile_definitions(gb_wasm PRIVATE USE_SDL)
endif()
target_compile_options(gb_wasm PRIVATE -s USE_SDL=2)
target_link_options(gb_wasm PRIVATE -sUSE_SDL=2)

target_include_directories(gb_wasm PUBLIC "../lib")
target_link_libraries(gb_wasm libgb hqx)
target_compile_options(gb_wasm PRIVATE -pthread -O3)

em_link_js_library(gb_wasm ${WASM_LIB})
target_link_options(gb_wasm PRIVATE --bind)
# target_link_options(gb_wasm PRIVATE --preload-file zelda.gbc)
target_link_options(gb_wasm PRIVATE -pthread)
target_link_options(gb_wasm PRIVATE -s INITIAL_MEMORY=134217728)
target_link_options(gb_wasm PRIVATE -sEXPORTED_RUNTIME_METHODS=["ccall"])