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

inline uint8_t get_color_id(uint8_t data1, uint8_t data2, uint8_t col)
{
	return (get_bit(data2, col) << 1) | ((uint8_t)get_bit(data1, col));
}

inline uint32_t get_color_u32(uint16_t col)
{
	uint8_t red = (col >> 0) & 0x1F;   // ((col & 0x1F) * 255) / 31;
	uint8_t green = (col >> 5) & 0x1F; // (((col >> 5) & 0x1F) * 255) / 31;
	uint8_t blue = (col >> 10) & 0x1F; // (((col >> 10) & 0x1F) * 255) / 31;

	red = (red << 3) | (red >> 2);
	green = (green << 3) | (green >> 2);
	blue = (blue << 3) | (blue >> 2);

	return (255 << 24) | (red << 16) | (green << 8) | blue;
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

PaletteData* const GPU::get_palette_data()
{
	return cgb_palettes;
}

void GPU::step(uint8_t cycles)
{
	uint8_t stat_val = memory->read_8bits(STAT) | 0x80;
	uint8_t curr_line = memory->read_8bits(LY);
	uint8_t prev_line = curr_line;

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
			if(clock_counter >= 204) //51 *4)
			{
				clock_counter -= 204;
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
			if(clock_counter >= 576)//114 *4)
			{
				clock_counter -= 576;
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
			if (clock_counter >= 80)//20 *4)
			{
				clock_counter -= 80;

				//We go into mode 3
				stat_val |= 0x3;

				if (get_bit(stat_val, 5))
					req_int = true;
			}
		break;
		// Pixel transfer
		case 3:
			if(clock_counter >= 172)//43 *4)
			{
				clock_counter -= 172;

				draw_scanline(curr_line);

				//We go into mode 0, Hblank
				stat_val &= 0xFC;
				if (get_bit(stat_val, 3))
					req_int = true;

				if (memory->cgb_enabled() && !(memory->read_8bits(0xFF55) & 0x80))
					memory->HDMATransfer(0, false);
			}
		break;
	}

	if(req_int)
		memory->request_interrupt(Memory::interrupt_id::STAT);

	//TODO : check if we really need to call this everytime
	memory->write_8bits(STAT, stat_val);
	memory->write_8bits(LY, curr_line);
	if(prev_line != curr_line)
		compareLYLYC();

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
	if(get_bit(lcd_control, 0) || memory->cgb_enabled())
	{
		draw_bg(line);

		if(get_bit(lcd_control, 5))
			draw_window(line);
	}


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

	uint8_t color_id = get_color_id(data1, data2, tile_pix_col);

	uint8_t palette = memory->read_8bits(PALETTE);
	color_id = (palette >> (color_id << 1)) & 0x03;

	uint8_t color = colors[color_id];

	// TODO : maybe rework
	pixels[row][col] = (255 << 24) | (color << 16) | (color << 8) | color;
}

void GPU::draw_tile_line(uint8_t row, uint8_t col, uint8_t tile_pix_row, uint16_t tile_addr, attributes* attr)
{
	const uint16_t PALETTE = 0xFF47;
	uint8_t palette = memory->read_8bits(PALETTE);
	bool x_flip = false;
	bool vram_bank = false;

	if(attr != NULL)
	{
		palette = attr->fields.cgb_palette;
		if(attr->fields.y_flip)
			tile_pix_row = 7 - tile_pix_row;

		x_flip = attr->fields.x_flip;
		vram_bank = attr->fields.vram_bank;
	}

	tile_pix_row *= 2; //each line is 2 bytes
	// uint16_t data = memory->read_16bits(tile_addr + tile_pix_row);
	// uint8_t data2 = (data & 0xFF00) >> 8;
	// uint8_t data1 = data & 0x00FF;
	uint8_t data1 = memory->read_vram(tile_addr + tile_pix_row, vram_bank);
	uint8_t data2 = memory->read_vram(tile_addr + tile_pix_row + 1, vram_bank);

	for(int pix_col = 0; pix_col < 8 ; pix_col++)
	{
		uint8_t screen_col = col + pix_col;

		if( screen_col >= 0 && screen_col < LCD_WIDTH)
		{
			uint8_t tile_pix_col = x_flip?pix_col:7 - pix_col; // pixel 0 is bit 7 etc, except if flipped

			uint8_t color_id = get_color_id(data1, data2, tile_pix_col);
			uint32_t color;

			if (attr != NULL)
			{
				uint16_t col = cgb_palettes[0].palette[palette][color_id].value;
				color = get_color_u32(col);
			}
			else
			{
				color_id = (palette >> (color_id << 1)) & 0x03;
				uint8_t col = colors[color_id];
				color = (255 << 24) | (col << 16) | (col << 8) | col;
			}


			// color_id = (palette >> (color_id << 1)) & 0x03;
			// uint8_t color = colors[color_id];

			// TODO : maybe rework
			pixels[row][screen_col] = color;//(255 << 24) | (color << 16) | (color << 8) | color;
			bg_color_prio_line[screen_col] = color_id;

			if(attr != NULL)
			{
				// We store prio flag in left nibble
				bg_color_prio_line[screen_col] |= (attr->fields.priority << 4);
			}
		}
	}
}

void GPU::draw_bg(uint8_t line)
{
	const uint16_t SCX = 0xFF43;
	const uint16_t SCY = 0xFF42;


	uint8_t lcd_control = memory->read_8bits(LCDC_C);

	uint8_t scroll_x = memory->read_8bits(SCX);
	uint8_t scroll_y = memory->read_8bits(SCY);

	bool use_unsigned = get_bit(lcd_control, 4);
	uint16_t tile_data_bank_addr = use_unsigned ? 0x8000 : 0x8800;

	uint16_t bg_map_addr = get_bit(lcd_control, 3) ? 0x9C00:0x9800;

	//row in pixels in the big background space
	uint8_t bg_y_tile = scroll_y + line;

	// each tile is 8x8 and each row is 32 block
	uint16_t tile_row_offset = (bg_y_tile/8) * 32;

	// pixel line inside the tile
	uint8_t pixel_line = bg_y_tile & 0x7; // tile is 8x8 so we take %8

	bool is_cgb = memory->cgb_enabled();
	attributes bg_map_attr;

	/*	We draw on a tile basis instead of pixel to only have to fetch
		the data from memory once per tile.
		So on the first tile we may be out of screen if scroll_x is in the middle of a tile
	*/
	for(int i = -(scroll_x & 0x7); i < LCD_WIDTH; i += 8)
	{
		uint8_t bg_x_tile = scroll_x + i;
		uint16_t tile_col_offset = bg_x_tile/8;

		uint16_t tile_addr = bg_map_addr + tile_row_offset + tile_col_offset;
		uint8_t tile_id = memory->read_vram(tile_addr, 0);

		if(is_cgb)
			bg_map_attr.value = memory->read_vram(tile_addr, 1);

		uint16_t tile_data_addr = 0;
		if(use_unsigned)
			tile_data_addr = tile_data_bank_addr + tile_id * 16;
		else
			tile_data_addr = tile_data_bank_addr + ((int8_t)tile_id + 128) * 16;

		draw_tile_line(line, i, pixel_line, tile_data_addr, is_cgb?&bg_map_attr:NULL);
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
	uint16_t tile_row_offset = (bg_y_tile / 8) * 32;
	uint8_t pixel_line = bg_y_tile & 0x7; // tile is 8x8 so we take %8

	bool is_cgb = memory->cgb_enabled();
	attributes bg_map_attr;

	for (int i = window_x - 7 ; i < LCD_WIDTH; i += 8)
	{
		uint8_t bg_x_tile = i - (window_x - 7) ;
		uint16_t tile_col_offset = bg_x_tile / 8;

		uint16_t tile_addr = bg_map_addr + tile_row_offset + tile_col_offset;
		uint8_t tile_id = memory->read_vram(tile_addr,0);

		if (is_cgb)
			bg_map_attr.value = memory->read_vram(tile_addr, 1);

		uint16_t tile_data_addr = 0;
		if (use_unsigned)
			tile_data_addr = tile_data_bank_addr + tile_id * 16;
		else
			tile_data_addr = tile_data_bank_addr + ((int8_t)tile_id + 128) * 16;

		draw_tile_line(line, i, pixel_line, tile_data_addr, is_cgb ? &bg_map_attr : NULL);
	}
}

void GPU::draw_objects(uint8_t line)
{
	const uint16_t OBJ0 = 0xFE00;
	const uint16_t OBJ_BANK = 0x8000;

	const bool is_cgb = memory->cgb_enabled();

	struct spr_attribute {
		uint8_t y_pos;
		uint8_t x_pos;
		uint8_t tile_number;
		attributes attr_flags;
	};

	uint8_t lcd_control = memory->read_8bits(LCDC_C);
	bool use_8x16 = get_bit(lcd_control, 2);

	// We loop backwards because the number with smallest obj number has highest priority
	for(int i = 39; i >= 0; i--)
	{
		const uint8_t spr_idx = i * 4;
		spr_attribute attr;
		memcpy(&attr, memory->get_data(OBJ0 + spr_idx), 4);
		// attr.y_pos = memory->read_8bits(OBJ0 + spr_idx);
		// attr.x_pos = memory->read_8bits(OBJ0 + spr_idx + 1);
		// attr.tile_number = memory->read_8bits(OBJ0 + spr_idx + 2);
		// attr.attr_flags.value = memory->read_8bits(OBJ0 + spr_idx + 3);

		uint8_t ysize = (use_8x16)?16:8;

		// If we are in the current scanline
		int16_t ajusted_y = attr.y_pos - 0x10;
		int16_t ajusted_x = attr.x_pos - 0x08;
		if(ajusted_y <= line && ajusted_y + ysize > line)
		{

			if(attr.x_pos == 0 || attr.x_pos >= 168)
				continue;

			uint8_t pixel_line = line - ajusted_y;
			if(get_bit(attr.attr_flags.value, 6))
				pixel_line = ysize - pixel_line -1 ;

			pixel_line *= 2;

			uint8_t data1 = memory->read_vram(OBJ_BANK + attr.tile_number * 16 + pixel_line, attr.attr_flags.fields.vram_bank);
			uint8_t data2 = memory->read_vram(OBJ_BANK + attr.tile_number * 16 + pixel_line + 1, attr.attr_flags.fields.vram_bank);
			uint8_t bg_palette = memory->read_8bits(0xFF47);
			uint8_t palette;

			if(is_cgb)
				palette = attr.attr_flags.fields.cgb_palette;
			else
				palette = memory->read_8bits(get_bit(attr.attr_flags.value, 4) ? 0xFF49 : 0xFF48);

			// Backward because smallest X had priority
			for(int x = 7; x >= 0; x--)
			{
				uint8_t pixel_col = x;
				if(!get_bit(attr.attr_flags.value, 5))
					pixel_col = 7 - pixel_col; // pixel 0 is bit 7 etc

				uint8_t color_id = get_color_id(data1, data2, pixel_col);

				if(color_id == 0)
					continue;

				if(ajusted_x + x < 0)
					continue;

				uint8_t bg_color_0 = bg_palette  & 0x03;

				// Priority check, if it's behind the bg we don't draw
				// TODO: overlapping
				// BG prio flag is stored in bits 0xF0. If DMG flag is 0 so is ok

				// If BG color is 0 we draw the sprite regardless of the priority flags as it's transparent
				if ((bg_color_prio_line[ajusted_x + x] & 0x0F) != 0) {
					//BG has priority so we don't draw this pixel
					if(bg_color_prio_line[ajusted_x + x] >> 4)
						continue;

					//If BG prio flag is 0, we do the same check in OAM
					if (get_bit(attr.attr_flags.value, 7))
						continue;
				}

				uint32_t color;
				if(is_cgb)
				{
					uint16_t col = cgb_palettes[1].palette[palette][color_id].value;
					color = get_color_u32(col);
				}
				else
				{
					color_id = (palette >> (color_id << 1)) & 0x03;
					uint8_t col = colors[color_id];
					color = (255 << 24) | (col << 16) | (col << 8) | col;
				}


				pixels[line][ajusted_x + x] = color;
			}
		}
	}

}


//#ifndef NDEBUG

void GPU::draw_full_bg(uint32_t *pixels) const
{
	const uint16_t PALETTE = 0xFF47;

	uint8_t lcd_control = memory->read_8bits(LCDC_C);
	uint16_t bg_map_addr = get_bit(lcd_control, 3) ? 0x9C00 : 0x9800;
	bool use_unsigned = get_bit(lcd_control, 4);
	uint16_t tile_data_bank_addr = use_unsigned ? 0x8000: 0x8800;

	bool is_cgb = memory->cgb_enabled();
	attributes bg_map_attr;

	for (int i = 0; i < 32; i++)
	{

		// uint8_t bg_y_tile =  i;

		// each tile is 8x8 and each row is 32 block
		// uint16_t tileRow = (bg_y_tile / 8) * 32;
		uint16_t tile_row_offset = i * 32;

		for(int j = 0; j < 32; j++)
		{
			// uint8_t bg_x_tile = j;
			// uint16_t tileCol = bg_x_tile / 8;
			uint16_t tile_col_offset = j;

			uint16_t tile_addr = bg_map_addr + tile_row_offset + tile_col_offset;
			uint8_t tile_id = memory->read_vram(tile_addr, 0);

			if (is_cgb)
				bg_map_attr.value = memory->read_vram(tile_addr, 1);

			uint16_t tile_data_addr = 0;
			if (use_unsigned)
				tile_data_addr = tile_data_bank_addr + tile_id * 16;
			else
				tile_data_addr = tile_data_bank_addr + ((int8_t)tile_id + 128) * 16;

			uint8_t palette = memory->read_8bits(PALETTE);
			bool x_flip = false;
			bool y_flip = false;
			bool vram_bank = false;

			if (is_cgb)
			{
				palette = bg_map_attr.fields.cgb_palette;
				y_flip = bg_map_attr.fields.y_flip;
				x_flip = bg_map_attr.fields.x_flip;
				vram_bank = bg_map_attr.fields.vram_bank;
			}

			for(int row = 0; row < 8; row++)
			{
				// uint8_t pixel_line = bg_y_tile & 0x7; // tile is 8x8 so we take %8
				// pixel_line *= 2;					  //each line is 2 bytes
				uint8_t pixel_line = y_flip?  7 - row : row;
				pixel_line *= 2;

				uint8_t data1 = memory->read_vram(tile_data_addr + pixel_line, vram_bank);
				uint8_t data2 = memory->read_vram(tile_data_addr + pixel_line + 1, vram_bank);

				for(int col = 0; col < 8; col++)
				{
					// uint8_t pixel_col = bg_x_tile & 0x7;
					// pixel_col = x_flip ? pixel_col : 7 - pixel_col; // pixel 0 is bit 7 etc
					uint8_t pixel_col = x_flip ? col : 7 - col; // pixel 0 is bit 7 etc

					uint8_t color_id = get_color_id(data1, data2, pixel_col);
					uint32_t color;

					if ( is_cgb)
					{
						uint16_t col = cgb_palettes[0].palette[palette][color_id].value;
						color = get_color_u32(col);
					}
					else
					{
						color_id = (palette >> (color_id << 1)) & 0x03;
						uint8_t col = colors[color_id];
						color = (255 << 24) | (col << 16) | (col << 8) | col;
					}
					// TODO : maybe rework
					pixels[(i * 8 + row) * 256 + (j * 8 + col)] = color;
				}
			}
		}
	}
}

void GPU::display_bg_tiles(uint32_t* pixels, bool bank) const
{
	const uint16_t PALETTE = 0xFF47;

	uint8_t lcd_control = memory->read_8bits(LCDC_C);
	const uint16_t tile_data_bank_addr = 0x8000;
	const bool use_unsigned = get_bit(lcd_control, 4);
	const bool use_8x16 = get_bit(lcd_control, 2);

	bool is_cgb = memory->cgb_enabled();

	for (int i = 0; i < 24; i++)
	{

		// uint8_t bg_y_tile = i;

		// // each tile is 8x8 and each row is 16 block
		// uint16_t tileRow = (bg_y_tile / 8) * 16;
		uint16_t tile_row_offset = i * 16;


		for (int j = 0; j < 16; j++)
		{
			// uint8_t bg_x_tile = j;
			// uint16_t tile_col_offset = bg_x_tile / 8;
			uint16_t tile_col_offset = j;

			uint8_t palette = memory->read_8bits(PALETTE);

			uint16_t tile_id = tile_row_offset + tile_col_offset;
			uint8_t in_use = 0;

			//We want to know if the tile is in use by either the BG or the OAM to color it
			if(is_cgb)
			{
				// const uint16_t bg_data = 0x9800;
				const uint16_t bg_data = get_bit(lcd_control, 3) ? 0x9C00 : 0x9800;
				const uint16_t OAM = 0xFE00;

				attributes cgb_attr;

				for(int i = 0; i < 1024 && !in_use; i++)
				{
					uint8_t u8_id = memory->read_vram(bg_data + i, 0);
					int16_t id = u8_id;
					if(!use_unsigned)
						id = (int8_t)u8_id + 256;

					if (tile_id == id )
					{
						cgb_attr.value = memory->read_vram(bg_data + i, 1);
						if(cgb_attr.fields.vram_bank == bank)
						{
							in_use = 1;
							palette = cgb_attr.fields.cgb_palette;
						}
					}
				}

				for(int i = 39; i >= 0; i--)
				{
					const uint8_t spr_idx = i * 4;
					uint8_t id;
					id = memory->read_8bits(OAM + spr_idx + 2);

					if(use_8x16 ? ((id & 0xFE) == tile_id || (id | 0x01 )== tile_id) :id == tile_id)
					{
						cgb_attr.value = memory->read_8bits(OAM + spr_idx + 3);
						if (cgb_attr.fields.vram_bank == bank)
						{
							in_use = 2;
							palette = cgb_attr.fields.cgb_palette;
						}
					}
				}

			}

			uint16_t tile_data_addr = 0;
			tile_data_addr = tile_data_bank_addr + tile_id * 16;

			for(int row = 0; row < 8; row++)
			{
				// uint8_t pixel_line = bg_y_tile & 0x7; // tile is 8x8 so we take %8
				// pixel_line *= 2;					  //each line is 2 bytes
				uint8_t pixel_line = row*2;
				uint8_t data1 = memory->read_vram(tile_data_addr + pixel_line, bank);
				uint8_t data2 = memory->read_vram(tile_data_addr + pixel_line + 1, bank);

				for(int col = 0; col < 8; col++)
				{
					// uint8_t pixel_col = bg_x_tile & 0x7;
					// pixel_col = 7 - pixel_col; // pixel 0 is bit 7 etc
					uint8_t pixel_col = 7 - col;
					uint8_t color_id = get_color_id(data1, data2, pixel_col);
					uint32_t color;

					if(in_use)
					{
						uint16_t col = cgb_palettes[in_use - 1].palette[palette][color_id].value;
						color = get_color_u32(col);
					}
					else
					{
						color_id = (palette >> (color_id << 1)) & 0x03;
						uint8_t col = colors[color_id];
						color = (255 << 24) | (col << 16) | (col << 8) | col;
					}

					// pixels[i * 128 + j] = color;// (255 << 24) | (col << 16) | (col << 8) | col;
					pixels[(i * 8 + row) * 128 + (j * 8 + col)] = color;


				}
			}
		}
	}
}

uint32_t GPU::get_palette_color(bool bg, uint8_t palette, uint8_t col_id) const
{
	uint32_t color_u32;
	if( memory->cgb_enabled())
	{
		uint16_t color = cgb_palettes[bg].palette[palette][col_id].value;
		color_u32 = get_color_u32(color);
	} else {
		//We only care about col_id here
		palette = memory->read_8bits(0xFF47);
		col_id = (palette >> (col_id << 1)) & 0x03;
		uint8_t color = colors[col_id];
		color_u32 = (255 << 24) | (color << 16) | (color << 8) | color;
	}

	return color_u32;
}

//#endif