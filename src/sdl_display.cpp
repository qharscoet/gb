#include "sdl_display.h"
#include "options.h"

#include "tinyfiledialogs/tinyfiledialogs.h"


extern emu_options options;

#define SCREEN_FPS 60
#define TICKS_PER_FRAME 1000 / SCREEN_FPS

#define BUFFER_SIZE 4096

SDL_Display::SDL_Display()
{
}

SDL_Display::~SDL_Display()
{
	// /!\ Do not forget to free all textures !

	//Destroy window
	SDL_DestroyRenderer(sdlRenderer);
	SDL_DestroyWindow(sdlWindow);
	sdlWindow = NULL;
	sdlRenderer = NULL;

	SDL_CloseAudioDevice(audio_dev);

	//Quit SDL
	SDL_Quit();
}

void SDL_Display::set_title(std::string str)
{
	SDL_SetWindowTitle(sdlWindow, str.c_str());
}

int SDL_Display::init()
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 0;
	}
	else
	{
		//Initialize window and renderer, and check for NULL

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

	init_audio();

	return 1;
}

void SDL_Display::clear()
{
	SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 0);
	SDL_RenderClear(sdlRenderer);
}

void SDL_Display::update(const uint32_t* pixels)
{
	while((curr_time = SDL_GetTicks()) - prev_time < TICKS_PER_FRAME)
	{

		SDL_Delay(TICKS_PER_FRAME - (curr_time - prev_time));
	}

	{
		void* argb_pixels;
		int pitch;
		SDL_LockTexture(sdlTexture, NULL, &argb_pixels, &pitch);

		memcpy(argb_pixels, pixels, LCD_WIDTH * LCD_HEIGHT * sizeof(uint32_t));

		SDL_UnlockTexture(sdlTexture);
		SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);

		prev_time = curr_time;
	}
}

void SDL_Display::render()
{
	SDL_RenderPresent(sdlRenderer);
}

void SDL_Display::update_keystate()
{
	const Uint8 *state = SDL_GetKeyboardState(NULL);

	const uint8_t keys[8] = { SDL_SCANCODE_DOWN,SDL_SCANCODE_UP,SDL_SCANCODE_LEFT,SDL_SCANCODE_RIGHT,
								SDL_SCANCODE_RETURN, SDL_SCANCODE_BACKSPACE, SDL_SCANCODE_X, SDL_SCANCODE_C};
	keystate = 0;

	for(int i = 0; i < 8; i++)
	{
		if(state[keys[i]])
			keystate |= (1 << (7- i));
	}
}

uint8_t SDL_Display::get_keystate()
{
	return keystate;
}

bool SDL_Display::handle_events(Emulator &emu)
{
	SDL_Event event;

	if (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT
        || (event.type == SDL_WINDOWEVENT
			&& event.window.event == SDL_WINDOWEVENT_CLOSE
        	&& event.window.windowID == SDL_GetWindowID(sdlWindow)))
		{
				emu.quit();
				return 0;
		}

		if(event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_P)
			options.pause = !options.pause;

		if(event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_F1)
		{
			options.debug_ui = !options.debug_ui;
			options.display_changed = true;
		}

		if(event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_R)
			emu.reset();

		if(event.type == SDL_KEYDOWN && event.key.keysym.scancode == SDL_SCANCODE_O)
		{
			char const *lFilterPatterns[1] = {"*.gb"};
			char const *selection = tinyfd_openFileDialog( // there is also a wchar_t version
				"Open ROM",									// title
				NULL,										// optional initial directory
				1,											// number of filter patterns
				lFilterPatterns,							// char const * lFilterPatterns[2] = { "*.txt", "*.jpg" };
				NULL,										// optional filter description
				0											// forbid multiple selections
			);

			emu.save();
			emu.set_rom_file(selection);
			emu.reset();
		}

	}

	update_keystate();

	return 1;
}


int SDL_Display::init_audio()
{
	if (SDL_Init(SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 0;
	}

	SDL_AudioSpec want, have;

	SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
	want.freq = 48000;
	want.format = AUDIO_U8;
	want.channels = 1;
	want.samples = BUFFER_SIZE;
	want.callback = nullptr; /* you wrote this function elsewhere -- see SDL_AudioSpec for details */

	audio_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
	if (audio_dev == 0)
	{
		SDL_Log("Failed to open audio: %s", SDL_GetError());
	}
	else
	{
		if (have.format != want.format)
		{ /* we let this one thing change. */
			SDL_Log("We didn't get U8 audio format.");
		}
		SDL_PauseAudioDevice(audio_dev, 0); /* start audio playing. */
											//SDL_Delay(5000);			  /* let the audio callback play some sound for 5 seconds. */
	}

	return 1;
}

void SDL_Display::play_audio(const uint8_t *samples)
{
	while ((SDL_GetQueuedAudioSize(1)) > BUFFER_SIZE * sizeof(uint8_t))
	{
		SDL_Delay(1);
	}

	if(SDL_QueueAudio(audio_dev, samples, BUFFER_SIZE) == -1)
	{
		std::cout << SDL_GetError() << std::endl;
	}
}