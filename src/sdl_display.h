#ifndef __SDL_DISPLAY_H__
#define __SDL_DISPLAY_H__

#include "display.h"

#include <SDL2/SDL.h>
#undef main

class SDL_Display : public Display
{
private:
	static const int SCREEN_WIDTH = 640;
	static const int SCREEN_HEIGHT = 576;

	SDL_Texture* sdlTexture;
	SDL_Renderer *sdlRenderer;
	SDL_Window *sdlWindow;

	uint32_t prev_time;
	uint32_t curr_time;

	enum class keys {
		A,B,START,SELECT,
		RIGHT,LEFT,UP,DOWN,
	};

	uint8_t keystate;

	void update_keystate();

	SDL_AudioDeviceID audio_dev;
	int init_audio();
public:
	SDL_Display();
	~SDL_Display();

	void set_title(std::string str);

	int init();

	void clear();
	void update(const uint32_t *pixels);
	void render();

	bool handle_events(Emulator &emu);
	uint8_t get_keystate();

	void play_audio(const float *samples);
};

#endif
