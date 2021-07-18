#include "wasm_display.h"
#include <emscripten.h>

#include <hqx.h>

// #include "tinyfiledialogs/tinyfiledialogs.h"
// #include "hqx/hqx.h"

#define SCREEN_FPS 60
#define TICKS_PER_FRAME 1000 / SCREEN_FPS

#define BUFFER_SIZE 1024

extern "C" {
    extern void canvas_update(const uint32_t* pixels);
	extern void set_event_callback();
	extern uint8_t fetch_keystates();
	extern uint8_t fetch_size_multiplier();
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
	EM_ASM(document.title = UTF8ToString($0), str.c_str());
}


int WASM_Display::display_init()
{

	hqxInit();
	set_event_callback();

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

		uint32_t* render_pixels;
		if(size_multiplier != 1){
			render_pixels = new uint32_t[LCD_WIDTH * size_multiplier * LCD_HEIGHT * size_multiplier];
			switch(size_multiplier)
			{
				case 2: hq2x_32(pixels, render_pixels, LCD_WIDTH, LCD_HEIGHT); break;
				case 3: hq3x_32(pixels, render_pixels, LCD_WIDTH, LCD_HEIGHT); break;
				case 4: hq4x_32(pixels, render_pixels, LCD_WIDTH, LCD_HEIGHT); break;
				default:break;
			}
			pixels = render_pixels;
		}
        // canvas_update(pixels);
		EM_ASM_ARGS({
			var ctx = Module['canvas'].getContext('2d');
			ctx.canvas.height = 144 * 4;
			ctx.canvas.width = 160 * 4;
			ctx.imageSmoothingEnabled = false;

			if (!this.emu_canvas)
			{
				this.emu_canvas = document.createElement("canvas");
			}
			emu_canvas.width = 160 * $1;
			emu_canvas.height = 144 * $1;

			const pixel_array = new Uint8ClampedArray(HEAPU8.buffer, $0, 160 * 144 * 4 * $1 * $1);
			var image_data = new ImageData(new Uint8ClampedArray(pixel_array), 160 * $1, 144 * $1);
			emu_canvas.getContext('2d').putImageData(image_data, 0, 0);

			ctx.drawImage(emu_canvas, 0, 0, ctx.canvas.width, ctx.canvas.height);
		}, pixels, size_multiplier);

		if(size_multiplier != 1){
			delete[] render_pixels;
		}
	}
}

void WASM_Display::render()
{
}

void WASM_Display::update_keystate()
{
	keystate = fetch_keystates();
}

uint8_t WASM_Display::get_keystate()
{
	return keystate;
}

bool WASM_Display::handle_events(Emulator &emu)
{
	size_multiplier = fetch_size_multiplier();

	update_keystate();

	return 1;
}

void WASM_Display::switch_size(int multiplier)
{
	// Empty
}