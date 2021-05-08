Experimenting with wasm and emscripten.
Files here have been compiled locally and comitted here until proper integration.

Compiled with:
```
mkdir lib
cd lib
emcmake cmake ../lib/
emmake
cd ..
emcc main_wasm.cpp sdl_display.cpp sdl_audio.cpp tinyfiledialogs.o lib/libgb_lib.a  -o gb.js -std=c++20 -s USE_SDL=2 -pthread -s ASSERTIONS=1 -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_RUNTIME_METHODS='["ccall"]' -O3
```

And somme stuff commented out in the SDL Code lie SDL_Delays