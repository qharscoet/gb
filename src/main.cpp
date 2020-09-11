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

#ifndef NDEBUG
	Debug_Display display(emu);
#else
	SDL_Display display;
#endif

	emu.init();
	display.init();

	std::string rom_filename = "";
	if(argc > 1) {
		rom_filename = argv[1];
		emu.reset();
	}

	while(!emu.is_exiting())
	{
		display.handle_events(emu);

		if(emu.needs_reload())
		{
			if(!emu.load_rom(rom_filename))
			{
				std::cout << "Error opening file " << argv[1] << std::endl;
				emu.stop();
			}
			else
			{
				emu.start();
				display.set_title(emu.get_game_name());
			}
		}
		else
		{
			std::chrono::duration<double, std::milli> t_emu, t_display, t_total;
			auto total_start = std::chrono::steady_clock::now();
			auto start = total_start;

			if (emu.is_running())
			{
				emu.step(display.get_keystate());
			}

			auto end = std::chrono::steady_clock::now();
			t_emu = end - start;

			start = std::chrono::steady_clock::now();

			display.clear();
			display.update(emu.get_pixel_data());
			display.render();

			end = std::chrono::steady_clock::now();
			t_display = end - start;

			auto total_end = std::chrono::steady_clock::now();
			t_total = total_end - total_start;

			// std::cout << " emu time " << t_emu.count() << " ms "<< "\n";
			// std::cout << " display time " << t_display.count() << " ms" << "\n";

			// std::cout << "total time " <<  t_total.count() << " ms " << std::endl;
		}
	}

	return 0;
}
