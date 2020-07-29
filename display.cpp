#include "display.h"

Display::Display(/* args */)
{
}

Display::~Display()
{
}

int Display::init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 0;
	}
	else
	{
		//Initialize window and renderer, and check for NULL
		//sdlWindow = SDL_CreateWindow("My swaggy emulator ! ", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);

		SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &sdlWindow, &sdlRenderer);
		if (sdlWindow == NULL)
		{
			std::cout << "Window could not be created! SDL_Error:" << SDL_GetError() << std::endl;
			return 0;
		}

		if (sdlRenderer == NULL)
		{
			std::cout <<  "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
			return 0;
		}

		SDL_SetWindowTitle(sdlWindow, "My swaggy emulator");
	}

	return 1;
}

void Display::free()
{
	// /!\ Do not forget to free all textures !

	//Destroy window
	SDL_DestroyRenderer(sdlRenderer);
	SDL_DestroyWindow(sdlWindow);
	sdlWindow = NULL;
	sdlRenderer = NULL;

	//Quit SDL
	SDL_Quit();
}

void Display::clear()
{
	SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 0);
	SDL_RenderClear(sdlRenderer);
}

void Display::update()
{
	SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
	for (int i = 0; i < SCREEN_WIDTH; ++i)
		SDL_RenderDrawPoint(sdlRenderer, i, i);
}

void Display::render()
{
	SDL_RenderPresent(sdlRenderer);
}

bool Display::handle_events()
{
	SDL_Event event;

	if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
		return 0;

	return 1;
}