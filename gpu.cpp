#include "gpu.h"


inline bool get_bit(uint8_t val, uint8_t b)
{
	return val & (1 << b);
}

inline uint8_t set_bit(uint8_t val, uint8_t b)
{
	return val | (1 << b);
}

inline uint8_t res_bit(uint8_t val, uint8_t b)
{
	return val & ~(1 << b);
}

GPU::GPU(Memory* memory)
{
	this->memory = memory;
}

GPU::~GPU()
{
}

uint8_t* GPU::get_pixel_data()
{
	return pixels[0];
}


void GPU::step(uint8_t cycles)
{
	const uint16_t LCDC_C = 0xFF40;


	uint8_t stat_val = memory->read_8bits(STAT);
	uint8_t curr_line = memory->read_8bits(LY);

	//LCD is disabled
	if(!get_bit(memory->read_8bits(LCDC_C), 7))
	{
		//We force it to Mode 1
		stat_val = (stat_val & 0xFC) | 0x1;
		memory->write_8bits(STAT, stat_val);
		memory->write_8bits(LY, 0);

		clock_counter = 0;

		//Do not display anything
		return;
	}

	clock_counter += cycles;

	uint8_t lcd_mode = stat_val & 0x3;
	bool req_int = false;

	switch(lcd_mode){
		//H-blank
		case 0:
			if(clock_counter >= 51)
			{
				clock_counter -= 51;
				curr_line++;

				if(curr_line == 144)
				{
					//Go into Vblank
					stat_val |= 0x1; // mode is at 00 so OR 01 gives us mode 1
					memory->request_interrupt(Memory::interrupt_id::VBLANK);

					if(get_bit(stat_val, 4))
						req_int = true;


				} else
				{
					stat_val |= 0x2; // mode is at 00 so OR 10 gives us mode 2
					if (get_bit(stat_val, 5))
						req_int = true;
				}

				//memory->write_8bits(LY, curr_line);

			}
		break;
		// Vblank
		case 1:
			if(clock_counter >= 114)
			{
				clock_counter -= 114;
				curr_line++;

				if(curr_line == 154)
				{
					curr_line = 0;
					//We go into mode 2
					set_bit(stat_val, 1);
					res_bit(stat_val, 0);

					if (get_bit(stat_val, 5))
						req_int = true;
				}
			}

		break;
		// OAM read
		case 2:
			if (clock_counter >= 20)
			{
				clock_counter -= 20;

				//We go into mode 2
				stat_val |= 0x3;

				if (get_bit(stat_val, 5))
					req_int = true;
			}
		break;
		// Pixel transfer
		case 3:
			if(clock_counter >= 43)
			{
				clock_counter -= 43;

				draw_scanline(curr_line);

				//We go into mode 0, Hblank
				stat_val &= 0xFC0;
				if (get_bit(stat_val, 3))
					req_int = true;

			}
		break;
	}

	if(req_int)
		memory->request_interrupt(Memory::interrupt_id::STAT);

	//TODO : check if we really need to call this everytime
	compareLYLYC();

	memory->write_8bits(STAT, stat_val);
	memory->write_8bits(LY, curr_line);
}

void GPU::compareLYLYC()
{
	const uint16_t LYC = 0xFF45;

	uint8_t ly_val = memory->read_8bits(LY);
	uint8_t lyc_val = memory->read_8bits(LYC);
	uint8_t stat_val = memory->read_8bits(STAT);

	if(ly_val == lyc_val)
	{
		set_bit(stat_val, 2);

		if(get_bit(stat_val, 6))
		{
			memory->request_interrupt(Memory::interrupt_id::STAT);
		}
	} else
	{
		res_bit(stat_val, 2);
	}

	memory->write_8bits(STAT, stat_val);
}

void GPU::draw_scanline(uint8_t line)
{
	for(int i = 0; i < LCD_WIDTH; i++)
	{
		pixels[line][i] = 128;
	}
}