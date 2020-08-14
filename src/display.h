#include <SDL2/SDL.h>
#undef main
#include <iostream>

#include "imgui/imgui_impl_sdl.h"
#include "emulator.h"

void debug_ui_init();
void debug_ui_free();
void debug_ui_render(const GPU& gpu);

class Display
{
private:
	const int SCREEN_WIDTH = 640;
	const int SCREEN_HEIGHT = 576;

	static const uint8_t LCD_WIDTH = 160;
	static const uint8_t LCD_HEIGHT = 144;

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

public:
	Display(/* args */);
	~Display();

	int init();
	void free();

	void clear();
	void update(const uint32_t* pixels, const GPU& gpu);
	void render(const GPU& gpu);

	bool handle_events();
	uint8_t get_keystate();
};