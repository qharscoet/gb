#include "memory.h"

Memory::Memory(/* args */)
{
	mmap = new char[MEMSIZE];
	std::memset(mmap, 0, MEMSIZE);
}

Memory::~Memory()
{
	delete[] mmap;
}

void Memory::DMATransfer(uint8_t src)
{
	uint16_t addr  = src << 8;
	memcpy(mmap + 0xFE00, mmap + addr, 160);
	//TODO : check for timings and stuff
}

void Memory::load_content(std::istream &file)
{
	file.read(mmap, 0x8000);
}

void Memory::load_content(const uint8_t* data, uint32_t size)
{
	memcpy(mmap, data, size);
}

uint8_t Memory::read_8bits(uint16_t addr) const
{
	//echo ram
	if(addr >= 0xE000 && addr < 0xFE00)
		addr -= 0x2000;

	return mmap[addr];
}

uint16_t Memory::read_16bits(uint16_t addr) const
{
	//echo ram
	if (addr >= 0xE000 && addr < 0xFE00)
		addr -= 0x2000;

	uint8_t lsb = mmap[addr++];
	uint8_t msb = mmap[addr++];

	return (uint16_t)(msb << 8) | (uint16_t)(lsb);
}

void Memory::write_8bits(uint16_t addr, uint8_t value)
{
	// this is ROM space and can't be written
	if(addr < 0x8000)
		return;

	//echo ram
	if (addr >= 0xE000 && addr < 0xFE00)
		addr -= 0x2000;

	if(addr == 0xFF00)
	{
		uint8_t curr_value = mmap[addr];
		curr_value &= 0xC0;
		curr_value |= (value & 0x30);

		switch(value & 0x30)
		{
			case 0x20:
				curr_value |= ((joypad_keys >> 4) & 0x0F);
				break;
			case 0x10:
				curr_value |= (joypad_keys & 0x0F);
				break;
			case 0x30:
				curr_value |= 0x0F;
				break;
		}

		mmap[addr] = curr_value;
		// mmap[addr] ^= 0xFF; //Flip all bits
	}else
	{
		//if(addr == 0xFF40 && disabling && we are in vblank WE CANT IT MAY DAMAGE THE HARDWARE
		mmap[addr] = value;

		//If we write to FF46 we trigger DMA
		if(addr == 0xFF46)
			DMATransfer(value);
	}
}

void Memory::write_16bits(uint16_t addr, uint16_t value)
{
	//echo ram
	if (addr >= 0xE000 && addr < 0xFE00)
		addr -= 0x2000;

	uint8_t msb = (value >> 8);
	uint8_t lsb = (value & 0xFF);

	mmap[addr] = lsb;
	mmap[addr + 1] = msb;
	//*((uint16_t *)(mmap + addr)) = value;
}

void Memory::request_interrupt(interrupt_id id)
{
	const uint16_t IF = 0xFF0F;
	uint8_t bit = static_cast<uint8_t>(id);
	uint8_t if_val = read_8bits(IF);

	if_val |= (1 << bit);

	write_8bits(IF, if_val);
}

void Memory::update_joypad(uint8_t keys)
{
	// In the gameboy, 0 means pressed
	joypad_keys = ~keys;
}

char *const Memory::get_data(uint16_t addr)
{
	return &mmap[addr];
}