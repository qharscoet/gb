#include <SDL2/SDL.h>
#include <iostream>


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

	uint32_t prev_time, curr_time;

public:
	Display(/* args */);
	~Display();

	int init();
	void free();

	void clear();
	void update(uint8_t* pixels);
	void render();

	bool handle_events();
};