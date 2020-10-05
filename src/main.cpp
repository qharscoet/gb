#include <cstdlib>
#include <iostream>
#include <chrono>

#include <cassert>
#ifdef _WIN32
#include <intrin.h>
#endif

#include "emulator.h"
#include "options.h"

#include "debug_display.h"
#include "sdl_display.h"

emu_options options;


void options_init()
{
	options.pause = false;

	options.sound.channel1 = true;
	options.sound.channel2 = true;
	options.sound.channel3 = true;
	options.sound.channel4 = true;
}

int main(int argc, char const *argv[])
{
	Emulator emu;
	Display *display;

#ifndef NDEBUG
	display = new Debug_Display(emu);
	options.debug_ui = true;
#else
	display = new SDL_Display();
	options.debug_ui = false;
#endif

	options_init();
	emu.init();
	display->init();

	if(argc > 1) {
		emu.set_rom_file(argv[1]);
	}

	while(!emu.is_exiting())
	{
		display->handle_events(emu);

		if(emu.needs_reload())
		{
			if(!emu.load_rom())
			{
				std::cout << "Error opening file " << argv[1] << std::endl;
				emu.stop();
			}
			else
			{
				emu.start();
				display->set_title(emu.get_game_name());
			}
		}
		else
		{
			std::chrono::duration<double, std::milli> t_emu, t_display, t_total;
			auto total_start = std::chrono::steady_clock::now();
			auto start = total_start;
			auto end = std::chrono::steady_clock::now();

			if (emu.is_running())
			{

				const uint16_t CYCLES_BY_FRAME = 17556;
				uint16_t cycles_total = 0;

				start = std::chrono::steady_clock::now();
				if (!options.pause)
				{
					while (cycles_total < CYCLES_BY_FRAME)
					{

						cycles_total += emu.step(display->get_keystate());

						const float *samples = emu.get_audio_data();
						if (samples != nullptr)
						{
							display->play_audio(samples);
							emu.clear_audio();
						}
					}
				}
			}

			end = std::chrono::steady_clock::now();
			t_emu = end - start;

			display->clear();
			display->update(emu.get_pixel_data());
			display->render();

			end = std::chrono::steady_clock::now();
			t_display = end - start;

			auto total_end = std::chrono::steady_clock::now();
			t_total = total_end - total_start;

			// std::cout << " emu time " << t_emu.count() << " ms "<< "\n";
			// std::cout << " display time " << t_display.count() << " ms" << "\n";

			// std::cout << "total time " <<  t_total.count() << " ms " << std::endl;
		}

		if (options.display_changed)
		{
			delete display;

			if (options.debug_ui)
				display = new Debug_Display(emu);
			else
				display = new SDL_Display();

			display->init();
			display->set_title(emu.get_game_name());

			options.display_changed = false;
		}
	}

	delete display;
	return 0;
}
