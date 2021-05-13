#include <cstdlib>
#include <iostream>
#include <memory>

#include <cassert>
#ifdef _WIN32
#include <intrin.h>
#endif

#include "../lib/emulator.h"
#include <emscripten.h>

#include "wasm_display.h"
#include "../src/sdl_audio.h"
#include "../src/sdl_display.h"

// #define SDL_MAIN_HANDLED

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
#if USE_SDL
SDL_Display display;
#else
WASM_Display display;
#endif
SDL_Audio audio;

extern "C" {
    extern void play_samples(const float *samples);
}

int i = 0;

void loop()
{
		// std::cout << "looping " << i++ << std::endl;
		display.handle_events(emu);
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
				display.set_title(emu.get_game_name());
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

						cycles_total += emu.step(display.get_keystate());

						const float *samples = emu.get_audio_data();
						if (samples != nullptr)
						{
							audio.play_audio(samples);
							emu.clear_audio();
						}
					}

					emu.update_rtc();
				}
			}
		}
		display.clear();
		display.update(emu.get_pixel_data());
		display.render();
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
	options.debug_ui = false;

// #if USE_SDL
// 	display = new SDL_Display();
// #else
// 	display = new WASM_Display();
// #endif
// 	audio = new SDL_Audio();

	options_init();
	emu.init();
	display.display_init();
	audio.audio_init();

	// emu.set_rom_file("zelda.gbc");
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
		display.set_title(emu.get_game_name());
	}

	std::cout << "going to main loop" << std::endl;
	emscripten_set_main_loop(loop, 0, 1);

	// emscripten_request_animation_frame_loop(loop,0);

	// delete display;
	// delete audio;

	return 0;
}

#if defined(__EMSCRIPTEN__)
	using namespace emscripten;
	EMSCRIPTEN_BINDINGS(Emulator_module) {
		class_<Emulator>("Emulator")
			.constructor<>()
			.function("init", &Emulator::init)
			.function("is_gameboy_color", &Emulator::is_gameboy_color)
			.function("init", &Emulator::init)
			.function("reset", &Emulator::reset)
			.function("start", &Emulator::start)
			.function("stop", &Emulator::stop)
			.function("load_rom", select_overload<bool()>(&Emulator::load_rom))
			.function("needs_reload", &Emulator::needs_reload)
			.function("is_running", &Emulator::is_running)
			.function("is_exiting", &Emulator::is_exiting)
			.function("get_game_name", &Emulator::get_game_name)
			.function("set_rom_file", &Emulator::set_rom_file)
			.function("step", &Emulator::step)
			.function("get_pixel_data", &Emulator::get_pixel_data, allow_raw_pointers())
			.function("draw_full_bg", &Emulator::draw_full_bg, allow_raw_pointers())
			.function("draw_bg_tiles", &Emulator::draw_bg_tiles, allow_raw_pointers())
			.function("get_audio_data", &Emulator::get_audio_data, allow_raw_pointers())
			.function("clear_audio", &Emulator::clear_audio)
			.function("update_rtc", &Emulator::update_rtc)
			;
	}
#endif