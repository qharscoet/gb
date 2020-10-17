#include "display.h"

#include <SDL2/SDL.h>
#undef main

#include "emulator.h"

#include "sdl_audio.h"


class Debug_Display : public Display, public SDL_Audio
{
private:
	SDL_GLContext gl_context;
	SDL_Window *sdlWindow;

	Emulator& emu;

	enum class keys {
		A,B,START,SELECT,
		RIGHT,LEFT,UP,DOWN,
	};

	uint8_t keystate;

	void update_keystate();

	SDL_AudioDeviceID audio_dev;

public:
	Debug_Display(Emulator& emu);
	~Debug_Display();

	void set_title(std::string str);

	int display_init();
	// int audio_init();

	void clear();
	void update(const uint32_t *pixels);
	void render();

	bool handle_events(Emulator &emu);
	uint8_t get_keystate();

	void play_audio(const float *samples);
};