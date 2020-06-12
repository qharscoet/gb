#include "cpu.h"

void CPU::inc(r16 r)
{
	write_r16(r, read_r16(r) + 1);
}

void CPU::dec(r16 r)
{
	write_r16(r, read_r16(r) - 1);
}

// load value into register
void CPU::ld(r8 r, uint8_t value)
{
	write_r8(r, value);
}

// load register into register
void CPU::ld(r8 r1, r8 r2)
{
	write_r8(r1, read_r8(r2));
}

// load adress in r2 to r1
void CPU::ld(r8 r1, r16 r2)
{
	write_r8(r1, memory->read_8bits(read_r16(r2)));
}

// load r2 into adress stored in r1
void CPU::ld(r16 r1, r8 r2)
{
	memory->write_8bits(read_r16(r1), read_r8(r2));
}

// load value stored at addr in r
void CPU::ld(r8 r, a16 addr)
{
	write_r8(r, memory->read_8bits(addr));
}

// load r into addr
void CPU::ld(a16 addr, r8 r)
{
	memory->write_8bits(addr, read_r8(r));
}


// 16 bits load
void CPU::ld(r16 r, uint16_t value)
{
	write_r16(r, value);
}

void CPU::ld(r16 r1, r16 r2)
{
	write_r16(r1, read_r16(r2));
}




void CPU::ldhl(r16 r, int8_t n)
{
	int8_t val = read_r16(r) + n;
	write_r16(r16::HL, val);

	reset_flag(flag_id::Z);
	reset_flag(flag_id::N);

}

void CPU::ldd(r8 r1, r16 r2)
{
	ld(r1, r2);
	dec(r2);
}

void CPU::ldd(r16 r1, r8 r2)
{
	ld(r1, r2);
	dec(r1);
}

void CPU::ldi(r8 r1, r16 r2)
{
	ld(r1,r2);
	inc(r2);
}

void CPU::ldi(r16 r1, r8 r2)
{
	ld(r1,r2);
	inc(r1);
}
