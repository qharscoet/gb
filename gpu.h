#ifndef _GPU_H_
#define _GPU_H_

#include "memory.h"

class GPU
{
private:
	static const uint8_t LCD_WIDTH = 160;
	static const uint8_t LCD_HEIGHT = 144;

	uint8_t pixels[LCD_HEIGHT][LCD_WIDTH];

	Memory *memory;

public:
	GPU(Memory* memory);
	~GPU();

	void step();
};

#endif