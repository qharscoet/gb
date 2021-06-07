#include "wasm_display.h"
#include <emscripten.h>

// #include "tinyfiledialogs/tinyfiledialogs.h"
// #include "hqx/hqx.h"

#define SCREEN_FPS 60
#define TICKS_PER_FRAME 1000 / SCREEN_FPS

#define BUFFER_SIZE 1024

extern "C" {
    extern void canvas_update(const uint32_t* pixels);
	extern void audio_visualizer_draw();
}

WASM_Display::WASM_Display()
{
}

WASM_Display::~WASM_Display()
{
	// /!\ Do not forget to free all textures !
}

void WASM_Display::set_title(std::string str)
{
	// SDL_SetWindowTitle(sdlWindow, str.c_str());
}

int WASM_Display::display_init()
{

	// hqxInit();
	// init_audio();

	return 1;
}

void WASM_Display::clear()
{
}

void WASM_Display::update(const uint32_t* pixels)
{
	{
		void* argb_pixels;
		int pitch;

		// uint32_t* render_pixels;
		// if(size_multiplier != 1){
		// 	render_pixels = new uint32_t[LCD_WIDTH * size_multiplier * LCD_HEIGHT * size_multiplier];
		// 	switch(size_multiplier)
		// 	{
		// 		case 2: hq2x_32(pixels, render_pixels, LCD_WIDTH, LCD_HEIGHT); break;
		// 		case 3: hq3x_32(pixels, render_pixels, LCD_WIDTH, LCD_HEIGHT); break;
		// 		case 4: hq4x_32(pixels, render_pixels, LCD_WIDTH, LCD_HEIGHT); break;
		// 		default:break;
		// 	}
		// 	pixels = render_pixels;
		// }
        // canvas_update(pixels);
		EM_ASM_ARGS({
			var ctx = Module['canvas'].getContext('2d');
			ctx.canvas.height = 144 * 4;
			ctx.canvas.width = 160 * 4;
			;
			ctx.imageSmoothingEnabled = false;

			if (!this.emu_canvas)
			{
				this.emu_canvas = document.createElement("canvas");
				emu_canvas.width = 160;
				emu_canvas.height = 144;
			}

			const pixel_array = new Uint8ClampedArray(HEAPU8.buffer, $0, 160 * 144 * 4);
			var image_data = new ImageData(new Uint8ClampedArray(pixel_array), 160, 144);
			emu_canvas.getContext('2d').putImageData(image_data, 0, 0);

			ctx.drawImage(emu_canvas, 0, 0, ctx.canvas.width, ctx.canvas.height);
		}, pixels);

		audio_visualizer_draw();

		// if(size_multiplier != 1){
		// 	delete[] render_pixels;
		// }
	}
}

void WASM_Display::render()
{
}

void WASM_Display::update_keystate()
{
	// const Uint8 *state = SDL_GetKeyboardState(NULL);

	// const uint8_t keys[8] = { SDL_SCANCODE_DOWN,SDL_SCANCODE_UP,SDL_SCANCODE_LEFT,SDL_SCANCODE_RIGHT,
	// 							SDL_SCANCODE_RETURN, SDL_SCANCODE_BACKSPACE, SDL_SCANCODE_X, SDL_SCANCODE_C};
	// keystate = 0;

	// for(int i = 0; i < 8; i++)
	// {
	// 	if(state[keys[i]])
	// 		keystate |= (1 << (7- i));
	// }
}

uint8_t WASM_Display::get_keystate()
{
	return keystate;
}

bool WASM_Display::handle_events(Emulator &emu)
{
	// SDL_Event event;

	// if (SDL_PollEvent(&event))
	// {
	// 	if (event.type == SDL_QUIT
    //     || (event.type == SDL_WINDOWEVENT
	// 		&& event.window.event == SDL_WINDOWEVENT_CLOSE
    //     	&& event.window.windowID == SDL_GetWindowID(sdlWindow)))
	// 	{
	// 			emu.quit();
	// 			return 0;
	// 	}

	// 	if(event.type == SDL_DROPFILE)
	// 	{
	// 		char *filename = event.drop.file;
	// 		if (std::strcmp(strrchr(filename, '.'), ".gb") == 0 ||
	// 			std::strcmp(strrchr(filename, '.'), ".gbc") == 0)
	// 		{
	// 			emu.save();
	// 			emu.set_rom_file(filename);
	// 			emu.reset();
	// 		} else {
	// 			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error opening file", "Please drop a .gb file", sdlWindow);
	// 		}

	// 		SDL_free(filename);
	// 	}

	// 	if(event.type == SDL_KEYDOWN)
	// 	{
	// 		switch(event.key.keysym.scancode)
	// 		{
	// 			case SDL_SCANCODE_P:
	// 				options.pause = !options.pause;
	// 				break;
	// 			case SDL_SCANCODE_F1:
	// 				options.debug_ui = !options.debug_ui;
	// 				options.display_changed = true;
	// 				break;
	// 			case SDL_SCANCODE_R:
	// 				emu.reset();
	// 				break;
	// 			case SDL_SCANCODE_O: {
	// 				char const *lFilterPatterns[2] = {"*.gb", "*.gbc"};
	// 				char const *selection = tinyfd_openFileDialog( // there is also a wchar_t version
	// 					"Open ROM",									// title
	// 					NULL,										// optional initial directory
	// 					2,											// number of filter patterns
	// 					lFilterPatterns,							// char const * lFilterPatterns[2] = { "*.txt", "*.jpg" };
	// 					NULL,										// optional filter description
	// 					0											// forbid multiple selections
	// 				);

	// 				if (selection)
	// 				{
	// 					emu.save();
	// 					emu.set_rom_file(selection);
	// 					emu.reset();
	// 				}
	// 			}
	// 			break;
	// 			case SDL_SCANCODE_KP_1:
	// 				switch_size(1);
	// 				break;
	// 			case SDL_SCANCODE_KP_2:
	// 				switch_size(2);
	// 				break;
	// 			case SDL_SCANCODE_KP_3:
	// 				switch_size(3);
	// 				break;
	// 			case SDL_SCANCODE_KP_4:
	// 				switch_size(4);
	// 				break;


	// 			case SDL_SCANCODE_0:
	// 				emu.listen_network();
	// 				break;
	// 			case SDL_SCANCODE_1:
	// 				emu.connect_network();
	// 				break;
	// 			default:
	// 				break;
	// 		}
	// 	}
	// }

	update_keystate();

	return 1;
}

void WASM_Display::switch_size(int multiplier)
{
	if(size_multiplier != multiplier)
	{
		// size_multiplier = multiplier;
		// SDL_DestroyTexture(sdlTexture);
		// sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, LCD_WIDTH * size_multiplier, LCD_HEIGHT * size_multiplier);

		// if (sdlTexture == NULL)
		// {
		// 	std::cout << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
		// 	return;
		// }
	}
}