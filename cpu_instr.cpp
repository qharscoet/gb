#include "cpu.h"

#define GET_BIT(c, bit) (c & (1 << bit))

void CPU::set_inc_flags(uint8_t val)
{
	//uint8_t val = read_r8(r);

	set_flag(flag_id::Z, (uint8_t)(val + 1) == 0);

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

	set_flag(flag_id::Z,val - 1 == 0);

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
	int16_t val = read_r16(r);


	reset_flag(flag_id::Z);
	reset_flag(flag_id::N);

	if ((uint32_t)(val + n) & 0x10000)
		set_flag(flag_id::C);

	if (((val & 0xfff) + (n & 0xfff)) & 0x1000)
		set_flag(flag_id::H);


	write_r16(r16::HL, val + n);
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
	dec(r16::SP);
	memory->write_16bits(read_r16(r16::SP), read_r16(r));
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

	bool carry = (uint16_t)(r_val + val) & 0x100;
	set_flag(flag_id::C, carry);

	bool half_carry = ((r_val & 0xf) + (val & 0xf)) & 0x10;
	set_flag(flag_id::H, half_carry);

	r_val += val;

	set_flag(flag_id::Z, val == 0);

	write_r8(r, r_val);
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

	set_flag(flag_id::N);

	//carry
	set_flag(flag_id::C,r_val < val);

	//half_carry
	set_flag(flag_id::H, (r_val & 0xf) < (val & 0xf));

	r_val -= val;

	set_flag(flag_id::Z, r_val == 0);

	write_r8(r, r_val);
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

	set_flag(flag_id::Z, r_val == 0);

	reset_flag(flag_id::N);
	set_flag(flag_id::H);
	reset_flag(flag_id::C);

	write_r8(r, r_val);
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

	set_flag(flag_id::Z, r_val == 0);

	reset_flag(flag_id::N);
	reset_flag(flag_id::H);
	reset_flag(flag_id::C);

	write_r8(r, r_val);
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

	set_flag(flag_id::Z, r_val == 0);

	reset_flag(flag_id::N);
	reset_flag(flag_id::H);
	reset_flag(flag_id::C);

	write_r8(r, r_val);
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

	set_flag(flag_id::Z, r_val == val);

	set_flag(flag_id::N);

	set_flag(flag_id::C, r_val < val);

	set_flag(flag_id::H, (r_val & 0xf) < (val & 0xf) );
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

	bool carry = (uint32_t)(r_val + val) & 0x10000;
	set_flag(flag_id::C, carry);

	bool half_carry = ((r_val & 0xfff) + (val & 0xfff)) & 0x1000;
	set_flag(flag_id::H, half_carry);

	r_val += val;

	write_r16(r, r_val);
}

void CPU::add(r16 r1, r16 r2)
{
	add(r1, read_r16(r2));
}


uint8_t CPU::swap(uint8_t val)
{
	uint8_t lsb = val & 0xf;
	val = (lsb << 4) | (val >> 4);

	set_flag(flag_id::Z,val == 0);

	reset_flag(flag_id::N);
	reset_flag(flag_id::H);
	reset_flag(flag_id::C);

	return val;
}


// template<std::variant<CPU::r8, CPU::r16> p>
// void CPU::swap()
// {
// 	write_8bits(p, swap(read_8bits(p)));
// }



void CPU::daa()
{
	uint8_t val = read_r8(r8::A);
	uint8_t lsb = val & 0xf;
	uint8_t msb = (val & 0xf0) >> 4;

	//TODO: test if correct behavior
	if(get_flag(flag_id::N)) // if last op was add
	{
		if(lsb > 9 || get_flag(flag_id::H))
		{
			val += 0x6;
		}

		if(msb > 9 || get_flag(flag_id::C))
		{
			val += 0x60;
			set_flag(flag_id::C);
		}
	} else
	{
		if(get_flag(flag_id::C))
		{
			val -= 0x60;
			set_flag(flag_id::C);
		}
		if(get_flag(flag_id::H))
		{
			val -= 0x6;
		}
	}

	set_flag(flag_id::Z, val == 0);

	reset_flag(flag_id::H);

	write_r8(r8::A, val);
}

void CPU::cpl()
{
	uint8_t val = read_r8(r8::A);
	val = ~val;
	write_r8(r8::A, val);

	set_flag(flag_id::N);
	set_flag(flag_id::H);
}

void CPU::ccf()
{
	if( get_flag(flag_id::C))
	{
		reset_flag(flag_id::C);
	} else
	{
		set_flag(flag_id::C);
	}

	reset_flag(flag_id::N);
	reset_flag(flag_id::H);
}

void CPU::scf()
{
	reset_flag(flag_id::N);
	reset_flag(flag_id::H);
	set_flag(flag_id::C);
}

void CPU::halt()
{
	//TODO
}

void CPU::stop()
{
	//TODO
}

void CPU::di()
{
	ime = false;
}

void CPU::ei()
{
	ime_scheduled = true;
}


/* Rotate and shifts */

uint8_t CPU::rotate(uint8_t val, bool left, bool carry)
{
	bool dropped_bit = left ? GET_BIT(val, 7) : GET_BIT(val, 0) ;
	bool new_bit0 = carry ? dropped_bit : get_flag(flag_id::C);

	set_flag(flag_id::C, dropped_bit);

	if(left)
		val = (val << 1) | new_bit0;
	else
		val = (val >> 1) | (new_bit0 << 7);

	return val;

}

void CPU::rotate_a(bool left, bool carry)
{
	uint8_t val = rotate(read_r8(r8::A), left, carry);

	reset_flag(flag_id::Z);
	reset_flag(flag_id::N);
	reset_flag(flag_id::H);

	write_r8(r8::A, val);
}

void CPU::rotate_r(r8 r, bool left, bool carry)
{
	uint8_t val = rotate(read_r8(r), left, carry);

	set_flag(flag_id::Z, val == 0);

	reset_flag(flag_id::N);
	reset_flag(flag_id::H);

	write_r8(r, val);
}

void CPU::rotate_p(uint16_t addr, bool left, bool carry)
{
	uint8_t val = memory->read_8bits(addr);
	val = rotate(val, left, carry);

	set_flag(flag_id::Z, val == 0);

	reset_flag(flag_id::N);
	reset_flag(flag_id::H);

	memory->write_8bits(addr, val);
}


void CPU::rlca()
{
	rotate_a(true, true);
}

void CPU::rla()
{
	rotate_a(true, false);
}

void CPU::rrca()
{
	rotate_a(false, true);

}

void CPU::rra()
{
	rotate_a(false, false);
}



uint8_t  CPU::shift(uint8_t val, bool left, bool arithmetic)
{
	bool last_bit = left ?  GET_BIT(val, 7) : GET_BIT(val, 0);

	if(left)
	{
		val <<= 1;
	} else {
		if(arithmetic) {
			//Hack to keep the msb by using the fact that g++ keeps the sign when shifting
			val = (uint8_t)((int8_t)val >> 1);
		} else {
			val >>= 1;
		}
	}

	set_flag(flag_id::C, last_bit);

	set_flag(flag_id::Z, val == 0);

	reset_flag(flag_id::N);
	reset_flag(flag_id::H);

	return val;
}

void CPU::bit(uint8_t val, uint8_t b)
{
	if(GET_BIT(val, b))
		reset_flag(flag_id::Z);
	else
		set_flag(flag_id::Z);

	reset_flag(flag_id::N);
	set_flag(flag_id::H);
}

void CPU::jp()
{
	*pc = read_pc16();
}

void CPU::jnz()
{
	uint16_t nn = read_pc16();
	if ((test_true = !get_flag(flag_id::Z)))
		*pc = nn;
}

void CPU::jz()
{
	uint16_t nn = read_pc16();
	if ((test_true = get_flag(flag_id::Z)))
		*pc = nn;
}

void CPU::jnc()
{
	uint16_t nn = read_pc16();
	if ((test_true = !get_flag(flag_id::C)))
		*pc = nn;
}

void CPU::jc()
{
	uint16_t nn = read_pc16();
	if ((test_true = get_flag(flag_id::C)))
		*pc = nn;
}

void CPU::jhl()
{
	*pc = read_r16(r16::HL);
}

void CPU::jr()
{
	int8_t e = read_pc8();
	*pc += e;
}

void CPU::jrnz()
{
	int8_t e = read_pc8();
	if ((test_true = !get_flag(flag_id::Z)))
		*pc += e;
}

void CPU::jrz()
{
	int8_t e = read_pc8();
	if ((test_true = get_flag(flag_id::Z)))
		*pc += e;
}

void CPU::jrnc()
{
	int8_t e = read_pc8();
	if ((test_true = !get_flag(flag_id::C)))
		*pc += e;
}

void CPU::jrc()
{
	int8_t e = read_pc8();
	if ((test_true = get_flag(flag_id::C)))
		*pc += e;
}

//Calls

void CPU::call_addr(uint16_t addr)
{
	dec(r16::SP);
	dec(r16::SP);
	memory->write_16bits(read_r16(r16::SP), *pc);
	*pc = addr;
}

void CPU::call()
{
	uint16_t addr = read_pc16();
	call_addr(addr);
}

void CPU::callnz()
{
	uint16_t addr = read_pc16();
	if ((test_true = !get_flag(flag_id::Z)))
	{
		call_addr(addr);
	}
}

void CPU::callz()
{
	uint16_t addr = read_pc16();
	if ((test_true = get_flag(flag_id::Z)))
	{
		call_addr(addr);
	}
}

void CPU::callnc()
{
	uint16_t addr = read_pc16();
	if ((test_true = !get_flag(flag_id::C)))
	{
		call_addr(addr);
	}
}

void CPU::callc()
{
	uint16_t addr = read_pc16();
	if ((test_true = get_flag(flag_id::C)))
	{
		call_addr(addr);
	}
}

void CPU::rst(uint8_t n)
{
	dec(r16::SP);
	dec(r16::SP);
	memory->write_16bits(read_r16(r16::SP), *pc);
	*pc = n;
}

void CPU::ret()
{
	*pc = memory->read_16bits(read_r16(r16::SP));
	inc(r16::SP);
	inc(r16::SP);
}

void CPU::retnz()
{
	if((test_true = !get_flag(flag_id::Z)))
		ret();

}

void CPU::retz()
{
	if ((test_true = get_flag(flag_id::Z)))
		ret();

}

void CPU::retnc()
{
	if ((test_true = !get_flag(flag_id::C)))
		ret();

}

void CPU::retc()
{
	if ((test_true = get_flag(flag_id::C)))
		ret();

}

void CPU::reti()
{
	ret();
	ime = true;
}




// #define GEN_TEMPLATES(name) \
//  	template void CPU::name<CPU::r8::A>(); \
// 	template void CPU::name<CPU::r8::B>(); \
// 	template void CPU::name<CPU::r8::C>(); \
// 	template void CPU::name<CPU::r8::D>(); \
// 	template void CPU::name<CPU::r8::E>(); \
// 	template void CPU::name<CPU::r8::H>(); \
// 	template void CPU::name<CPU::r8::L>(); \
// 	template void CPU::name16<CPU::r16::HL>();


// GEN_TEMPLATES(swap)
// GEN_TEMPLATES(rlc)
// GEN_TEMPLATES(rl)
// GEN_TEMPLATES(rrc)
// GEN_TEMPLATES(rr)
// GEN_TEMPLATES(sla)
// GEN_TEMPLATES(sra)
// GEN_TEMPLATES(srl)