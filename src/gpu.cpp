#include "gpu.h"


inline bool get_bit(uint8_t val, uint8_t b)
{
	return val & (1 << b);
}

inline void set_bit(uint8_t& val, uint8_t b)
{
	val =  val | (1 << b);
}

inline void res_bit(uint8_t& val, uint8_t b)
{
	val = val & ~(1 << b);
}

GPU::GPU(Memory* memory)
{
	this->memory = memory;
	clock_counter = 0;
}

GPU::~GPU()
{
}

const uint32_t* GPU::get_pixel_data() const
{
	return pixels[0];
}


void GPU::step(uint8_t cycles)
{
	uint8_t stat_val = memory->read_8bits(STAT);
	uint8_t curr_line = memory->read_8bits(LY);

	//LCD is disabled
	if(!get_bit(memory->read_8bits(LCDC_C), 7))
	{
		//We force it to Mode 0
		stat_val = (stat_val & 0xFC);// | 0x1;
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

				//We go into mode 3
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
	// for(int i = 0; i < LCD_WIDTH; i++)
	// {
	// 	pixels[line][i] = 128;
	// }
	uint8_t lcd_control = memory->read_8bits(LCDC_C);
	if(get_bit(lcd_control, 0))
		draw_bg(line);

	if(get_bit(lcd_control, 5))
		draw_window(line);

	if(get_bit(lcd_control, 1))
		draw_objects(line);
}

void GPU::draw_pixel(uint8_t row, uint8_t col, uint8_t tile_pix_row, uint8_t tile_pix_col, uint16_t tile_addr)
{
	const uint16_t PALETTE = 0xFF47;

	tile_pix_row *= 2; //each line is 2 bytes

	uint8_t data1 = memory->read_8bits(tile_addr + tile_pix_row);
	uint8_t data2 = memory->read_8bits(tile_addr + tile_pix_row + 1);

	tile_pix_col = 7 - tile_pix_col; // pixel 0 is bit 7 etc

	uint8_t color_id = (get_bit(data2, tile_pix_col) << 1) | (get_bit(data1, tile_pix_col));

	uint8_t palette = memory->read_8bits(PALETTE);
	color_id = (palette >> (color_id << 1)) & 0x03;

	uint8_t color = colors[color_id];

	// TODO : maybe rework
	pixels[row][col] = (255 << 24) | (color << 16) | (color << 8) | color;
}

void GPU::draw_bg(uint8_t line)
{
	const uint16_t SCX = 0xFF43;
	const uint16_t SCY = 0xFF42;


	uint8_t lcd_control = memory->read_8bits(LCDC_C);

	uint8_t scroll_x = memory->read_8bits(SCX);
	uint8_t scroll_y = memory->read_8bits(SCY);

	// static const uint8_t colors[4] = { 64, 128, 192, 255};
	// TODO : fix color
	//TODO : tests enabled bidule

	bool use_unsigned = get_bit(lcd_control, 4);
	uint16_t tile_data_bank_addr = use_unsigned ? 0x8000 : 0x8800;

	uint16_t bg_map_addr = get_bit(lcd_control, 3) ? 0x9C00:0x9800;

	//row in pixels in the big background space
	uint8_t bg_y_tile = scroll_y + line;

	// each tile is 8x8 and each row is 32 block
	uint16_t tile_row_offset = (bg_y_tile/8) * 32;

	// pixel line inside the tile
	uint8_t pixel_line = bg_y_tile & 0x7; // tile is 8x8 so we take %8

	for(int i = 0; i < LCD_WIDTH; i++)
	{
		uint8_t bg_x_tile = scroll_x + i;
		uint16_t tile_col_offset = bg_x_tile/8;

		uint16_t tile_addr = bg_map_addr + tile_row_offset + tile_col_offset;
		uint8_t tile_id = memory->read_8bits(tile_addr);

		uint16_t tile_data_addr = 0;
		if(use_unsigned)
			tile_data_addr = tile_data_bank_addr + tile_id * 16;
		else
			tile_data_addr = tile_data_bank_addr + ((int8_t)tile_id + 128) * 16;

		uint8_t pixel_col = bg_x_tile & 0x7;

		draw_pixel(line, i, pixel_line, pixel_col, tile_data_addr);
	}

}

void GPU::draw_window(uint8_t line)
{
	const uint16_t WY = 0xFF4A;
	const uint16_t WX = 0xFF4B;

	uint8_t lcd_control = memory->read_8bits(LCDC_C);

	uint8_t window_x = memory->read_8bits(WX);
	uint8_t window_y = memory->read_8bits(WY);

	if(line < window_y)
		return;

	bool use_unsigned = get_bit(lcd_control, 4);
	uint16_t tile_data_bank_addr = use_unsigned ? 0x8000 : 0x8800;


	uint16_t bg_map_addr = get_bit(lcd_control, 6) ? 0x9C00 : 0x9800;

	uint8_t bg_y_tile = line - window_y;

	// each tile is 8x8 and each row is 32 block
	uint16_t tileRow = (bg_y_tile / 8) * 32;
	uint8_t pixel_line = bg_y_tile & 0x7; // tile is 8x8 so we take %8

	for (int i = window_x - 7; i < LCD_WIDTH; i++)
	{
		uint8_t bg_x_tile = i - window_x;
		uint16_t tileCol = bg_x_tile / 8;

		uint16_t tile_addr = bg_map_addr + tileRow + tileCol;
		uint8_t tile_id = memory->read_8bits(tile_addr);

		uint16_t tile_data_addr = 0;
		if (use_unsigned)
			tile_data_addr = tile_data_bank_addr + tile_id * 16;
		else
			tile_data_addr = tile_data_bank_addr + ((int8_t)tile_id + 128) * 16;

		uint8_t pixel_col = bg_x_tile & 0x7;

		draw_pixel(line, i, pixel_line, pixel_col, tile_data_addr);
	}
}

void GPU::draw_objects(uint8_t line)
{
	const uint16_t OBJ0 = 0xFE00;
	const uint16_t OBJ_BANK = 0x8000;

	struct spr_attribute {
		uint8_t y_pos;
		uint8_t x_pos;
		uint8_t tile_number;
		uint8_t attr_flags;
	};

	uint8_t lcd_control = memory->read_8bits(LCDC_C);
	bool use_8x16 = get_bit(lcd_control, 2);

	for(int i = 0; i < 40; i++)
	{
		const uint8_t spr_idx = i * 4;
		spr_attribute attr;
		attr.y_pos = memory->read_8bits(OBJ0 + spr_idx) - 0x10;
		attr.x_pos = memory->read_8bits(OBJ0 + spr_idx + 1) - 0x08;
		attr.tile_number = memory->read_8bits(OBJ0 + spr_idx + 2);
		attr.attr_flags = memory->read_8bits(OBJ0 + spr_idx + 3);

		uint8_t ysize = (use_8x16)?16:8;

		// If we are in the current scanline
		if(attr.y_pos <= line && attr.y_pos + ysize > line)
		{
			uint8_t pixel_line = line - attr.y_pos;
			if(get_bit(attr.attr_flags, 6))
				pixel_line = ysize - pixel_line;

			pixel_line *= 2;

			uint8_t data1 = memory->read_8bits(OBJ_BANK + attr.tile_number * 16 + pixel_line);
			uint8_t data2 = memory->read_8bits(OBJ_BANK + attr.tile_number * 16 + pixel_line + 1);

			for(int x = 0; x < 8; x++)
			{
				uint8_t pixel_col = x;
				if(!get_bit(attr.attr_flags, 5))
					pixel_col = 7 - pixel_col; // pixel 0 is bit 7 etc

				uint8_t color_id = (get_bit(data2, pixel_col) << 1) | (get_bit(data1, pixel_col));

				if(color_id == 0)
					continue;

				uint8_t palette = memory->read_8bits(get_bit(attr.attr_flags, 4)?0xFF49:0xFF48);
				color_id = (palette >> (color_id << 1)) & 0x03;

				uint8_t col = colors[color_id];

				pixels[line][attr.x_pos + x] = (255 << 24) | (col << 16) | (col << 8) | col;
			}
		}
	}

}


#ifndef NDEBUG

void GPU::draw_full_bg(uint32_t *pixels) const
{
	const uint16_t PALETTE = 0xFF47;
	const uint16_t bg_map_addr = 0x9800;

	uint8_t lcd_control = memory->read_8bits(LCDC_C);
	uint16_t tile_data_bank_addr = 0;
	bool use_unsigned = get_bit(lcd_control, 4);
	if (use_unsigned)
	{
		tile_data_bank_addr = 0x8000;
	}
	else
	{
		tile_data_bank_addr = 0x8800;
	}

	for (int i = 0; i < 256; i++)
	{

		uint8_t bg_y_tile =  i;

		// each tile is 8x8 and each row is 32 block
		uint16_t tileRow = (bg_y_tile / 8) * 32;

		for(int j = 0; j < 256; j++)
		{
			uint8_t bg_x_tile = j;
			uint16_t tileCol = bg_x_tile / 8;

			uint16_t tile_addr = bg_map_addr + tileRow + tileCol;
			uint8_t tile_id = memory->read_8bits(tile_addr);

			uint16_t tile_data_addr = 0;
			if (use_unsigned)
				tile_data_addr = tile_data_bank_addr + tile_id * 16;
			else
				tile_data_addr = tile_data_bank_addr + ((int8_t)tile_id + 128) * 16;

			uint8_t pixel_line = bg_y_tile & 0x7; // tile is 8x8 so we take %8
			pixel_line *= 2;					  //each line is 2 bytes

			uint8_t data1 = memory->read_8bits(tile_data_addr + pixel_line);
			uint8_t data2 = memory->read_8bits(tile_data_addr + pixel_line + 1);

			uint8_t pixel_col = bg_x_tile & 0x7;
			pixel_col = 7 - pixel_col; // pixel 0 is bit 7 etc

			uint8_t color_id = (get_bit(data2, pixel_col) << 1) | (get_bit(data1, pixel_col));

			uint8_t palette = memory->read_8bits(PALETTE);
			color_id = (palette >> (color_id << 1)) & 0x03;

			uint8_t col = colors[color_id];

			// TODO : maybe rework
			pixels[i * 256 + j] = (col << 24) | (col << 16) | (col << 8) | (255);
		}
	}
}

void GPU::display_bg_tiles(uint32_t* pixels) const
{
	const uint16_t PALETTE = 0xFF47;
	const uint16_t bg_map_addr = 0x9800;

	uint8_t lcd_control = memory->read_8bits(LCDC_C);
	uint16_t tile_data_bank_addr = 0;
	bool use_unsigned = get_bit(lcd_control, 4);

	tile_data_bank_addr = 0x8000;

	for (int i = 0; i < 256; i++)
	{

		uint8_t bg_y_tile = i;

		// each tile is 8x8 and each row is 32 block
		uint16_t tileRow = (bg_y_tile / 8) * 16;

		for (int j = 0; j < 128; j++)
		{
			uint8_t bg_x_tile = j;
			uint16_t tileCol = bg_x_tile / 8;

			uint16_t tile_addr = bg_map_addr + tileRow + tileCol;
			uint8_t tile_id = tileRow + tileCol; //memory->read_8bits(tile_addr);

			uint16_t tile_data_addr = 0;
			tile_data_addr = tile_data_bank_addr + tile_id * 16;

			uint8_t pixel_line = bg_y_tile & 0x7; // tile is 8x8 so we take %8
			pixel_line *= 2;					  //each line is 2 bytes

			uint8_t data1 = memory->read_8bits(tile_data_addr + pixel_line);
			uint8_t data2 = memory->read_8bits(tile_data_addr + pixel_line + 1);

			uint8_t pixel_col = bg_x_tile & 0x7;
			pixel_col = 7 - pixel_col; // pixel 0 is bit 7 etc

			uint8_t color_id = (get_bit(data2, pixel_col) << 1) | (get_bit(data1, pixel_col));

			uint8_t palette = memory->read_8bits(PALETTE);
			color_id = (palette >> (color_id << 1)) & 0x03;

			uint8_t col = colors[color_id];

			pixels[i * 128 + j] = (col << 24) | (col << 16) | (col << 8) | (255);
		}
	}
}

#endif