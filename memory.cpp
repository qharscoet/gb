#include "memory.h"

Memory::Memory(/* args */)
{
	std::memset(mmap, 0, MEMSIZE);
}

Memory::~Memory()
{
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
	mmap[addr] = value;
}

void Memory::write_16bits(uint16_t addr, uint16_t value)
{
	*((uint16_t *)(mmap + addr)) = value;
}