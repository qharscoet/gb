#include "display.h"

#define SCREEN_FPS 60
#define TICKS_PER_FRAME 1000 / SCREEN_FPS

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

		sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, LCD_WIDTH, LCD_HEIGHT);

		if (sdlTexture == NULL)
		{
			std::cout << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
			return 0;
		}

		prev_time = curr_time = SDL_GetTicks();
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

void Display::update(uint32_t* pixels)
{
	if ((curr_time = SDL_GetTicks()) - prev_time >= TICKS_PER_FRAME)
	{
		clear();
		// SDL_SetRenderDrawColor(sdlRenderer, 255, 0, 0, 255);
		// for (int i = 0; i < SCREEN_WIDTH; ++i)
		// 	SDL_RenderDrawPoint(sdlRenderer, i, i);
		void* argb_pixels;
		int pitch;
		SDL_LockTexture(sdlTexture, NULL, &argb_pixels, &pitch);
		// for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++)
		// {
		// 	((uint32_t *)argb_pixels)[i] = pixels[i]; //(255 << 24) | (pixval << 16) | (pixval << 8) | pixval;
		// 	// SDL_SetRenderDrawColor(sdlRenderer, pixval, pixval, pixval, 255);
		// 	// SDL_RenderDrawPoint(sdlRenderer, i % LCD_WIDTH, i / LCD_WIDTH);
		// }
		memcpy(argb_pixels, pixels, LCD_WIDTH * LCD_HEIGHT * sizeof(uint32_t));

		// SDL_UpdateTexture(sdlTexture, NULL, argb_pixels, 160 * sizeof(uint32_t));
		SDL_UnlockTexture(sdlTexture);
		SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);

		prev_time = curr_time;
		// std::cout << " FRAME -------------- \n";

		render();
	}
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