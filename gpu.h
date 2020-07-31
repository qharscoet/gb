#ifndef _GPU_H_
#define _GPU_H_

#include "memory.h"

class GPU
{
private:
	static const uint8_t LCD_WIDTH = 160;
	static const uint8_t LCD_HEIGHT = 144;

	static const uint16_t STAT = 0xFF41;
	static const uint16_t LY = 0xFF44;

	uint8_t pixels[LCD_HEIGHT][LCD_WIDTH];
	int16_t clock_counter;

	Memory *memory;

	void compareLYLYC();
	void draw_scanline(uint8_t line);

public:
	GPU(Memory* memory);
	~GPU();

	void step(uint8_t cycles);

	uint8_t* get_pixel_data();
};

#endif