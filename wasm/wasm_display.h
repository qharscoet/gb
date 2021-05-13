#ifndef __WASM_DISPLAY_H__
#define __WASM_DISPLAY_H__

#include "display.h"

class WASM_Display : public Display
{
private:
	static const int SCREEN_WIDTH = 640;
	static const int SCREEN_HEIGHT = 576;

	int size_multiplier = 1;

	uint32_t prev_time;
	uint32_t curr_time;

	enum class keys {
		A,B,START,SELECT,
		RIGHT,LEFT,UP,DOWN,
	};

	uint8_t keystate;

	void update_keystate();

	void switch_size(int multiplier);

	// SDL_AudioDeviceID audio_dev;
	// int init_audio();
public:
	WASM_Display();
	~WASM_Display();

	void set_title(std::string str);

	int display_init();

	void clear();
	void update(const uint32_t *pixels);
	void render();

	bool handle_events(Emulator &emu);
	uint8_t get_keystate();

	// void play_audio(const float *samples);
};

#endif
