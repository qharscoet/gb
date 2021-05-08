#include <cstdlib>
#include <iostream>
#include <memory>

#include <cassert>
#ifdef _WIN32
#include <intrin.h>
#endif

#include "../lib/emulator.h"
#include <emscripten.h>
#include <emscripten/html5.h>

// #define SDL_MAIN_HANDLED
#include "display.h"
#include "sdl_display.h"

#include "audio.h"
#include "sdl_audio.h"

void options_init()
{
	options.pause = false;
	options.cgb_disabled = false;

	options.sound.channel1 = true;
	options.sound.channel2 = true;
	options.sound.channel3 = true;
	options.sound.channel4 = true;
}

Emulator emu;
Display* display;
Audio* audio;

int i = 0;

void loop()
{
		// std::cout << "looping " << i++ << std::endl;
		display->handle_events(emu);
		if (emu.needs_reload())
		{
			if (!emu.load_rom())
			{
				std::cout << "Error opening file " << std::endl;
				emu.stop();
			}
			else
			{
				emu.start();
				display->set_title(emu.get_game_name());
			}
		} else {
			if (emu.is_running())
			{

				const uint32_t CYCLES_BY_FRAME = 17556 * 4;
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
		}
		display->clear();
		display->update(emu.get_pixel_data());
		display->render();
}

extern "C" {
	EMSCRIPTEN_KEEPALIVE void wasm_load_file(const char *filename)
	{
		std::cout <<"Opening : " << filename << std::endl;
		emu.set_rom_file(filename);
		emu.reset();
	}
}

int main(int argc, char const *argv[])
{

	display = new SDL_Display();
	audio = new SDL_Audio();

	options.debug_ui = false;

	options_init();
	emu.init();
	display->display_init();
	audio->audio_init();

	//emu.set_rom_file("pokemon.gbc");
	emu.reset();

	if (!emu.load_rom())
	{
		std::cout << "close rom" << std::endl;
		emu.stop();
	}
	else
	{
		std::cout << "open rom" << std::endl;

		emu.start();
		std::cout << "post start " << std::endl;
		display->set_title(emu.get_game_name());
	}

	std::cout << "going to main loop" << std::endl;
	emscripten_set_main_loop(loop, 0, 1);

	// emscripten_request_animation_frame_loop(loop,0);
	delete display;
	delete audio;

	return 0;
}
