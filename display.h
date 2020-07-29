#include <SDL2/SDL.h>
#include <iostream>


class Display
{
private:
	const int SCREEN_WIDTH = 800;
	const int SCREEN_HEIGHT = 600;

	SDL_Texture* sdlTexture;
	SDL_Renderer *sdlRenderer;
	SDL_Window *sdlWindow;

public:
	Display(/* args */);
	~Display();

	int init();
	void free();

	void clear();
	void update(/*array of pixels*/);
	void render();

	bool handle_events();
};