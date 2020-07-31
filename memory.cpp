#include "memory.h"

Memory::Memory(/* args */)
{
	std::memset(mmap, 0, MEMSIZE);
}

Memory::~Memory()
{
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

uint8_t Memory::read_8bits(uint16_t addr)
{
	return mmap[addr];
}

uint16_t Memory::read_16bits(uint16_t addr)
{
	uint8_t lsb = mmap[addr++];
	uint8_t msb = mmap[addr++];

	return (uint16_t)(msb << 8) | (uint16_t)(lsb);
}

void Memory::write_8bits(uint16_t addr, uint8_t value)
{
	//if(addr == 0xFF40 && disabling && we are in vblank WE CANT IT MAY DAMAGE THE HARDWARE
	mmap[addr] = value;

	//If we write to FF46 we trigger DMA
	if(addr == 0xFF46)
		DMATransfer(value);

}

void Memory::write_16bits(uint16_t addr, uint16_t value)
{
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