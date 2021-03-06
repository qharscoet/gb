#ifndef _GPU_H_
#define _GPU_H_

#include "memory.h"

class GPU
{
private:
	static const uint8_t LCD_WIDTH = 160;
	static const uint8_t LCD_HEIGHT = 144;

	static const uint16_t LCDC_C = 0xFF40;
	static const uint16_t STAT = 0xFF41;
	static const uint16_t LY = 0xFF44;

	static constexpr uint8_t colors[4] = {0xFF, 0xCC, 0x77, 0};

	union attributes
	{
		struct
		{
			uint8_t cgb_palette : 3;
			bool vram_bank : 1;
			bool dmg_palette : 1; //Used only for obj in non-CGB modes
			bool x_flip : 1;
			bool y_flip : 1;
			bool priority : 1;
		} fields;
		uint8_t value;
	};

	// 2 sets (BG & OBJ) of 8 4-colors palettes, each color on 2 bytes
	PaletteData cgb_palettes[2];

	uint32_t pixels[LCD_HEIGHT][LCD_WIDTH];
	uint16_t clock_counter;

	uint8_t bg_color_prio_line[LCD_WIDTH];

	Memory *memory;

	void compareLYLYC();

	void draw_pixel(uint8_t row, uint8_t col, uint8_t tile_pix_row, uint8_t tile_pix_col, uint16_t tile_addr);
	void draw_tile_line(uint8_t row, uint8_t col, uint8_t tile_pix_row, uint16_t tile_addr, attributes* attr);

	void draw_bg(uint8_t line);
	void draw_window(uint8_t line);
	void draw_objects(uint8_t line);

	void draw_scanline(uint8_t line);

public:
	GPU(Memory* memory);
	~GPU();

	void step(uint8_t cycles);

	const uint32_t* get_pixel_data() const;
	PaletteData* const get_palette_data();

// TODO: fix build to properly exclude debug_display in Release mode
//#ifndef NDEBUG
	void draw_full_bg(uint32_t *pixels) const;
	void display_bg_tiles(uint32_t *pixels, bool bank) const;
	uint32_t get_palette_color(bool bg, uint8_t palette, uint8_t col_id) const;
//#endif

};
#endif