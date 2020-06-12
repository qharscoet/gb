#include "cpu.h"

void CPU::ld(r8 r, uint8_t value)
{
	write_r8(r, value);
}

void CPU::ld(r8 r1, r8 r2)
{
	write_r8(r1, read_r8(r2));
}

void CPU::ld(r8 r1, r16 r2)
{
	write_r8(r1, memory->read_8bits(read_r16(r2)));
}

void CPU::ld(r16 r1, r8 r2)
{
	memory->write_8bits(read_r16(r1), read_r8(r2));
}

void CPU::ld(r8 r, a16 addr)
{
	write_r8(r, memory->read_8bits(addr));
}

void CPU::ld(a16 addr, r8 r)
{
	memory->write_8bits(addr, read_r8(r));
}
