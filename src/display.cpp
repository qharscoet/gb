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

	debug_ui_init();

	return 1;
}

void Display::free()
{

	debug_ui_free();
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

void Display::update(const uint32_t* pixels, const GPU& gpu)
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

		// std::cout << " FRAME -------------- " << curr_time - prev_time << " ms  \n";
		prev_time = curr_time;

		render(gpu);
	}
}

void Display::render(const GPU& gpu)
{
	debug_ui_render(gpu);
	SDL_RenderPresent(sdlRenderer);
}

void Display::update_keystate()
{
	const Uint8 *state = SDL_GetKeyboardState(NULL);

	const uint8_t keys[8] = { SDL_SCANCODE_DOWN,SDL_SCANCODE_UP,SDL_SCANCODE_LEFT,SDL_SCANCODE_RIGHT,
								SDL_SCANCODE_RETURN, SDL_SCANCODE_BACKSPACE, SDL_SCANCODE_X, SDL_SCANCODE_X};
	keystate = 0;

	for(int i = 0; i < 8; i++)
	{
		if(state[keys[i]])
			keystate |= (1 << (7- i));
	}
}

uint8_t Display::get_keystate()
{
	return keystate;
}

bool Display::handle_events()
{
	SDL_Event event;

	if (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT
        || (event.type == SDL_WINDOWEVENT
			&& event.window.event == SDL_WINDOWEVENT_CLOSE
        	&& event.window.windowID == SDL_GetWindowID(sdlWindow)))
		{
				return 0;
		}
	}

	update_keystate();
	ImGui_ImplSDL2_ProcessEvent(&event);

	return 1;
}