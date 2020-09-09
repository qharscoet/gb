#include <cstdlib>
#include <iostream>
#include <chrono>

#include <cassert>
#ifdef _WIN32
#include <intrin.h>
#endif

#include "emulator.h"
#include "options.h"

#ifndef NDEBUG
	#include "debug_display.h"
#else
	#include "sdl_display.h"
#endif

emu_options options;
int main(int argc, char const *argv[])
{
	Emulator emu;

	Display* display;
#ifndef NDEBUG
	display = new Debug_Display(emu);
#else
	display = new SDL_Display();
#endif

	emu.init();
	display->init();

// #ifndef NDEBUG
// 	debug_ui_init();
// #endif

	if(argc > 1) {
		if(!emu.load_rom(argv[1]))
		{
			std::cout << "Error opening file " << argv[1] << std::endl;
		}
		else
		{
			emu.start();
			display->set_title(emu.get_game_name());
			while(display->handle_events())
			{
				std::chrono::duration<double, std::milli> t_emu, t_display, t_total;
				auto total_start = std::chrono::steady_clock::now();
				auto start = total_start;

				emu.step(display->get_keystate());

				auto end = std::chrono::steady_clock::now();
				t_emu = end - start;

				start = std::chrono::steady_clock::now();

				display->clear();
				display->update(emu.get_pixel_data());
				display->render();

// #ifndef NDEBUG
// 				debug_ui_render(emu);
// #endif

				end = std::chrono::steady_clock::now();
				t_display = end - start;

				auto total_end = std::chrono::steady_clock::now();
				t_total = total_end - total_start;

				// std::cout << " emu time " << t_emu.count() << " ms "<< "\n";
				// std::cout << " display time " << t_display->count() << " ms" << "\n";

				// std::cout << "total time " <<  t_total.count() << " ms " << std::endl;
			}
		}

	} else {
		std::cout << "please indicate rom file" << std::endl;
	}

// #ifndef NDEBUG
// 	debug_ui_free();
// #endif

	return 0;
}
