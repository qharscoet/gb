#ifndef __DEBUG_UI_H__
#define __DEBUG_UI_H__

#include <SDL2/SDL.h>
#include "emulator.h"

struct debug_options
{
	bool pause;
};

extern debug_options options;

void debug_ui_init();
void debug_ui_free();
void debug_ui_render(Emulator &emu);
void debug_ui_update(SDL_Event* event);

#endif