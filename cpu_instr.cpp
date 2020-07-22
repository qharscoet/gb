#include "cpu.h"

void CPU::set_inc_flags(uint8_t val)
{
	//uint8_t val = read_r8(r);

	if(val + 1 == 0)
		set_flag(flag_id::Z);

	reset_flag(flag_id::N);

	//Check for overflow on bit 3
	if (((val & 0x7) + 1) & 0x8)
		set_flag(flag_id::H);

	//write_r8(r, val + 1);
}

// TODO: refactor this with proper parameter management
void CPU::inc(r8 r)
{
	uint8_t val = read_r8(r);
	set_inc_flags(val);
	write_r8(r, val +1 );
}

void CPU::incp(r16 r)
{
	uint16_t addr = read_r16(r);
	uint8_t val = memory->read_8bits(read_r16(r));
	set_inc_flags(val);
	memory->write_8bits(addr, val +1);
}

void CPU::inc(r16 r)
{
	write_r16(r, read_r16(r) + 1);
}

void CPU::set_dec_flags(uint8_t val)
{

	if (val - 1 == 0)
		set_flag(flag_id::Z);

	reset_flag(flag_id::N);

	if ((val & 0xf) < 1)
		set_flag(flag_id::H);

}

void CPU::dec(r8 r)
{
	uint8_t val = read_r8(r);
	set_dec_flags(val);
	write_r8(r, val - 1);
}

void CPU::decp(r16 r)
{
	uint16_t addr = read_r16(r);
	uint8_t val = memory->read_8bits(read_r16(r));
	set_dec_flags(val);
	memory->write_8bits(addr, val - 1);
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

void CPU::ld(uint16_t addr, r16 r )
{
	memory->write_16bits(addr, read_r16(r));
}



void CPU::ldhl(r16 r, int8_t n)
{
	int16_t val = read_r16(r) + n;
	write_r16(r16::HL, val);

	reset_flag(flag_id::Z);
	reset_flag(flag_id::N);
	//TODO: wtf carry and half carry

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

void CPU::push(r16 r)
{
	//TODO : check if correct behavior
	dec(r16::SP);
	memory->write_16bits(read_r16(r16::SP), read_r16(r));
	dec(r16::SP);
}

void CPU::pop(r16 r)
{
	write_r16(r, memory->read_16bits(read_r16(r16::SP)));
	inc(r16::SP);
	inc(r16::SP);
}

void CPU::add(r8 r, uint8_t val)
{
	uint8_t r_val = read_r8(r);

	reset_flag(flag_id::N);

	if ((uint16_t)(r_val + val) & 0x100)
		set_flag(flag_id::C);

	if (((r_val & 0xf) + (val & 0xf)) & 0x10)
		set_flag(flag_id::H);

	r_val += val;

	if (r_val == 0)
		set_flag(flag_id::Z);

	write_r8(r, val);
}

void CPU::add(r8 r1, r8 r2)
{
	add(r1, read_r8(r2));
}

void CPU::add(r8 r1, r16 r2)
{
	add(r1, memory->read_8bits(read_r16(r2)));
}

void CPU::addc(r8 r1, r8 r2)
{
	/* TODO : cast uint8 to uint16 to handle overflow ? Set flags from the potential +1 ? */
	uint8_t r_val = read_r8(r2);
	r_val += get_flag(flag_id::C);
	add(r1, r_val);
}

void CPU::sub(r8 r, uint8_t val)
{
	uint8_t r_val = read_r8(r);

	reset_flag(flag_id::N);

	if (r_val < val)
		set_flag(flag_id::C);

	if ((r_val & 0xf) < (val & 0xf))
		set_flag(flag_id::H);

	r_val -= val;

	if (r_val == 0)
		set_flag(flag_id::Z);

	write_r8(r, val);
}

void CPU::sub(r8 r1, r8 r2)
{
	sub(r1, read_r8(r2));
}

void CPU::sub(r8 r1, r16 r2)
{
	sub(r1, memory->read_8bits(read_r16(r2)));
}

void CPU::subc(r8 r1, r8 r2)
{
	uint8_t val = read_r8(r2);
	val += get_flag(flag_id::C);

	sub(r1, val);
}

void CPU::band(r8 r, uint8_t val)
{
	uint8_t r_val = read_r8(r);

	r_val &= val;

	if (r_val == 0)
		set_flag(flag_id::Z);

	reset_flag(flag_id::N);
	set_flag(flag_id::H);
	reset_flag(flag_id::C);
}

void CPU::band(r8 r, r8 r2)
{
	band(r, read_r8(r2));
}

void CPU::band(r8 r, r16 r2)
{
	band(r, memory->read_8bits(read_r16(r2)));
}


void CPU::bor(r8 r, uint8_t val)
{
	uint8_t r_val = read_r8(r);

	r_val |= val;

	if (r_val == 0)
		set_flag(flag_id::Z);

	reset_flag(flag_id::N);
	reset_flag(flag_id::H);
	reset_flag(flag_id::C);
}

void CPU::bor(r8 r, r8 r2)
{
	bor(r, read_r8(r2));
}

void CPU::bor(r8 r, r16 r2)
{
	bor(r, memory->read_8bits(read_r16(r2)));
}

void CPU::bxor(r8 r, uint8_t val)
{
	uint8_t r_val = read_r8(r);

	r_val ^= val;

	if (r_val == 0)
		set_flag(flag_id::Z);

	reset_flag(flag_id::N);
	reset_flag(flag_id::H);
	reset_flag(flag_id::C);
}

void CPU::bxor(r8 r, r8 r2)
{
	bxor(r, read_r8(r2));
}

void CPU::bxor(r8 r, r16 r2)
{
	bxor(r, memory->read_8bits(read_r16(r2)));
}

void CPU::cp(r8 r, uint8_t val)
{
	uint8_t r_val = read_r8(r);

	if( r_val == val)
		set_flag(flag_id::Z);

	set_flag(flag_id::N);

	if(r_val < val)
		set_flag(flag_id::C);

	if ((r_val & 0xf) < (val & 0xf))
		set_flag(flag_id::H);
}

void CPU::cp(r8 r, r8 r2)
{
	cp(r, read_r8(r2));
}

void CPU::cp(r8 r, r16 r2)
{
	cp(r, memory->read_8bits(read_r16(r2)));
}


// 16 bits ALU

void CPU::add(r16 r, uint16_t val)
{
	uint16_t r_val = read_r16(r);

	reset_flag(flag_id::N);

	if ((uint32_t)(r_val + val) & 0x10000)
		set_flag(flag_id::C);

	if (((r_val & 0xfff) + (val & 0xfff)) & 0x1000)
		set_flag(flag_id::H);

	r_val += val;

	write_r16(r, r_val);
}

void CPU::add(r16 r1, r16 r2)
{
	add(r1, read_r16(r2));
}