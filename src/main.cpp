#include <cstdlib>
#include <iostream>

#include <cassert>
#ifdef _WIN32
#include <intrin.h>
#endif

#include "emulator.h"
#include "options.h"

#define SDL_MAIN_HANDLED
#include "display.h"
#include "debug_display.h"
#include "sdl_display.h"


#include "audio.h"
#include "sdl_audio.h"

emu_options options;


void options_init()
{
	options.pause = false;
	options.cgb_disabled = false;

	options.sound.channel1 = true;
	options.sound.channel2 = true;
	options.sound.channel3 = true;
	options.sound.channel4 = true;
}

int main(int argc, char const *argv[])
{
	Emulator emu;
	std::shared_ptr<Display> display;
	std::shared_ptr<Audio> audio;

#ifndef NDEBUG
	display = std::make_shared<Debug_Display>(emu);
	audio = std::dynamic_pointer_cast<Audio>(display);
	options.debug_ui = true;
#else
	display = std::make_shared<SDL_Display>();
	audio = std::make_shared<SDL_Audio>();
	options.debug_ui = false;
#endif

	options_init();
	emu.init();
	display->display_init();
	audio->audio_init();

	if(argc > 1) {
		emu.set_rom_file(argv[1]);
		emu.reset();
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
			if (emu.is_running())
			{

				const uint32_t CYCLES_BY_FRAME = 17556 *4;
				uint32_t cycles_total = 0;

				if (!options.pause)
				{
					while (cycles_total < CYCLES_BY_FRAME)
					{

						cycles_total += emu.step(display->get_keystate());

						const float *samples = emu.get_audio_data();
						if (samples != nullptr)
						{
							audio->play_audio(samples);
							emu.clear_audio();
						}
					}

					emu.update_rtc();
				}
			}

			display->clear();
			display->update(emu.get_pixel_data());
			display->render();
		}

		if (options.display_changed)
		{
				display.reset();
				audio.reset();

			if (options.debug_ui)
			{
				display = std::make_shared<Debug_Display>(emu);
				audio = std::dynamic_pointer_cast<Audio>(display); //dynamic_cast<Audio *>(display);
			}
			else
			{
				display = std::make_shared<SDL_Display>();
				audio = std::make_shared<SDL_Audio>();
			}

			display->display_init();
			display->set_title(emu.get_game_name());

			audio->audio_init();

			options.display_changed = false;
		}
	}

	display.reset();
	audio.reset();

	return 0;
}
