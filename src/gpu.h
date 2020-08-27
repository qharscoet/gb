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

	static const uint8_t colors[4];

	uint32_t pixels[LCD_HEIGHT][LCD_WIDTH];
	uint16_t clock_counter;

	Memory *memory;

	void compareLYLYC();

	void draw_pixel(uint8_t row, uint8_t col, uint8_t tile_pix_row, uint8_t tile_pix_col, uint16_t tile_addr);

	void draw_bg(uint8_t line);
	void draw_window(uint8_t line);
	void draw_objects(uint8_t line);

	void draw_scanline(uint8_t line);

public:
	GPU(Memory* memory);
	~GPU();

	void step(uint8_t cycles);

	const uint32_t* get_pixel_data() const;
	void draw_full_bg(uint32_t *pixels) const;
	void display_bg_tiles(uint32_t *pixels) const;
};
#endif