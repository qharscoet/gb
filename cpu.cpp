#include "cpu.h"
#include "extended_set.h"
#include <cstdint>
#include <functional>

CPU::CPU(/* args */)
{
	sp = &registers[4];
	*sp = 0xFFFF;

	pc = &registers[5];
	*pc = 0x100;

	flags = ((uint8_t*)registers) + 1;

	ime = true;

}

CPU::CPU(Memory* memory)
{
	sp = &registers[4];
	*sp = 0xFFFF;

	pc = &registers[5];
	*pc = 0x45FB;

	flags = ((uint8_t*)registers) + 1;

	ime = true;

	this->memory = memory;
}

CPU::~CPU()
{
}


uint8_t CPU::read_r8(r8 r)
{
	return ((uint8_t*)registers)[static_cast<uint8_t>(r)];
}

uint16_t CPU::read_r16(r16 r)
{
	return ((uint16_t*)registers)[static_cast<uint8_t>(r)];
}

void CPU::write_r8(r8 r, uint8_t value)
{
	((uint8_t*)registers)[static_cast<uint8_t>(r)] = value;
}

void CPU::write_r16(r16 r, uint16_t value)
{
	((uint16_t*)registers)[static_cast<uint8_t>(r)] = value;
}


uint8_t CPU::read_8bits(Param8bits p)
{
	uint8_t val = 0;

	if(std::holds_alternative<r8>(p))
	{
		val = read_r8(std::get<r8>(p));
	} else if (std::holds_alternative<r16>(p))
	{
		val = memory->read_8bits(read_r16(std::get<r16>(p)));
	}

	return val;
}


void CPU::write_8bits(Param8bits p, uint8_t value)
{
	if(std::holds_alternative<r8>(p))
	{
		write_r8(std::get<r8>(p), value);
	} else if (std::holds_alternative<r16>(p))
	{
		memory->write_8bits(read_r16(std::get<r16>(p)), value);
	}
}


uint8_t CPU::read_pc8()
{
	uint8_t ret = memory->read_8bits((*pc));
	(*pc)++;

	return ret;
}

uint16_t CPU::read_pc16()
{
	uint16_t ret = memory->read_16bits(*pc);
	*pc += 2;

	return ret;
}

void CPU::set_flag(flag_id f)
{
	uint8_t bit = static_cast<uint8_t>(f);
	(*flags) |= (1 << bit);
}
void CPU::reset_flag(flag_id f)
{
	uint8_t bit = static_cast<uint8_t>(f);
	(*flags) &= ~(1 << bit);
}

bool CPU::get_flag(flag_id f)
{
	uint8_t bit = static_cast<uint8_t>(f);
	return (*flags) & (1 << bit);
}

void CPU::step()
{
	uint8_t opcode = read_pc8();

	// instructions list : http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
	//handle instruction
	switch(opcode) {

		/*  8 Bits Load  */

		// LD direct load
		case 0x06:	ld(r8::B, read_pc8());	break;
		case 0x0E:	ld(r8::C, read_pc8());	break;
		case 0x16:	ld(r8::D, read_pc8());	break;
		case 0x1E:	ld(r8::E, read_pc8());	break;
		case 0x26:	ld(r8::H, read_pc8());	break;
		case 0x2E:	ld(r8::L, read_pc8());	break;

		//LD A,n

		case 0x0A:	ld(r8::A, r16::BC);		break;
		case 0x1A:	ld(r8::A, r16::DE);		break;
		case 0xFA:	ld(r8::A, read_pc16());	break;
		case 0x3E:	ld(r8::A, read_pc8());	break;

		// LD n,A
		case 0x02:	ld(r16::BC, r8::A);		break;
		case 0x12:	ld(r16::DE, r8::A);		break;
		case 0xEA:	ld(read_pc16(), r8::A);	break;

		//LD A,(C)
		case 0xF2:	ld(r8::A, (a16)(0xFF00 + read_r8(r8::C))); break;

		// LD (C), A
		case 0xE2:	ld((a16)(0xFF00 + read_r8(r8::C)), r8::A); break;

		// LD A, (HLD) or LD A,(HL-) or LDD A, (HL)
		case 0x3A:	ldd(r8::A,r16::HL);		break;

		// LD (HLD), A or LD (HL-), A or LDD (HL), A
		case 0x32:	ldd(r16::HL, r8::A);	break;

		// Same with HL+
		case 0x2A:	ldi(r8::A, r16::HL);	break;
		case 0x22:	ldi(r16::HL, r8::A);	break;

		// LDH (n), A
		case 0xE0:	ld(r8::A, (a16)(0xFF00 + read_pc8()));	break;
		case 0xF0:	ld((a16)(0xFF00 + read_pc8()), r8::A);	break;

		// LD r1,r2
		case 0x40: 	ld(r8::B,r8::B);		break;
		case 0x41: 	ld(r8::B,r8::C);		break;
		case 0x42: 	ld(r8::B,r8::D);		break;
		case 0x43: 	ld(r8::B,r8::E);		break;
		case 0x44: 	ld(r8::B,r8::H);		break;
		case 0x45: 	ld(r8::B,r8::L);		break;
		case 0x46: 	ld(r8::B,r16::HL);		break;
		case 0x47: 	ld(r8::B,r8::A);		break;
		case 0x48: 	ld(r8::C,r8::B);		break;
		case 0x49: 	ld(r8::C,r8::C);		break;
		case 0x4A: 	ld(r8::C,r8::D);		break;
		case 0x4B: 	ld(r8::C,r8::E);		break;
		case 0x4C: 	ld(r8::C,r8::H);		break;
		case 0x4D: 	ld(r8::C,r8::L);		break;
		case 0x4E: 	ld(r8::C,r16::HL);		break;
		case 0x4F: 	ld(r8::C,r8::A);		break;
		case 0x50: 	ld(r8::D,r8::B);		break;
		case 0x51: 	ld(r8::D,r8::C);		break;
		case 0x52: 	ld(r8::D,r8::D);		break;
		case 0x53: 	ld(r8::D,r8::E);		break;
		case 0x54: 	ld(r8::D,r8::H);		break;
		case 0x55: 	ld(r8::D,r8::L);		break;
		case 0x56: 	ld(r8::D,r16::HL);		break;
		case 0x57: 	ld(r8::D,r8::A);		break;
		case 0x58: 	ld(r8::E,r8::B);		break;
		case 0x59: 	ld(r8::E,r8::C);		break;
		case 0x5A: 	ld(r8::E,r8::D);		break;
		case 0x5B: 	ld(r8::E,r8::E);		break;
		case 0x5C: 	ld(r8::E,r8::H);		break;
		case 0x5D: 	ld(r8::E,r8::L);		break;
		case 0x5E: 	ld(r8::E,r16::HL);		break;
		case 0x5F: 	ld(r8::E,r8::A);		break;
		case 0x60: 	ld(r8::H,r8::B);		break;
		case 0x61: 	ld(r8::H,r8::C);		break;
		case 0x62: 	ld(r8::H,r8::D);		break;
		case 0x63: 	ld(r8::H,r8::E);		break;
		case 0x64: 	ld(r8::H,r8::H);		break;
		case 0x65: 	ld(r8::H,r8::L);		break;
		case 0x66: 	ld(r8::H,r16::HL);		break;
		case 0x67: 	ld(r8::H,r8::A);		break;
		case 0x68: 	ld(r8::L,r8::B);		break;
		case 0x69: 	ld(r8::L,r8::C);		break;
		case 0x6A: 	ld(r8::L,r8::D);		break;
		case 0x6B: 	ld(r8::L,r8::E);		break;
		case 0x6C: 	ld(r8::L,r8::H);		break;
		case 0x6D: 	ld(r8::L,r8::L);		break;
		case 0x6E: 	ld(r8::L,r16::HL);		break;
		case 0x6F: 	ld(r8::L,r8::A);		break;
		case 0x70: 	ld(r16::HL,r8::B);		break;
		case 0x71: 	ld(r16::HL,r8::C);		break;
		case 0x72: 	ld(r16::HL,r8::D);		break;
		case 0x73: 	ld(r16::HL,r8::E);		break;
		case 0x74: 	ld(r16::HL,r8::H);		break;
		case 0x75: 	ld(r16::HL,r8::L);		break;
		case 0x77: 	ld(r16::HL,r8::A);		break;
		case 0x78: 	ld(r8::A,r8::B);		break;
		case 0x79: 	ld(r8::A,r8::C);		break;
		case 0x7A: 	ld(r8::A,r8::D);		break;
		case 0x7B: 	ld(r8::A,r8::E);		break;
		case 0x7C: 	ld(r8::A,r8::H);		break;
		case 0x7D: 	ld(r8::A,r8::L);		break;
		case 0x7E: 	ld(r8::A,r16::HL);		break;
		case 0x7F: 	ld(r8::A,r8::A);		break;

		/* 16-bits load */
		case 0x01:	ld(r16::BC, read_pc16());	break;
		case 0x11:	ld(r16::DE, read_pc16());	break;
		case 0x21:	ld(r16::HL, read_pc16());	break;
		case 0x31:	ld(r16::SP, read_pc16());	break;
		case 0xF9:	ld(r16::SP, r16::HL);		break;
		case 0xF8:	ldhl(r16::SP, read_pc8());	break;
		case 0x08:	ld(read_pc16(), r16::SP);	break;

		case 0xF5:	push(r16::AF);	break;
		case 0xC5:	push(r16::BC);	break;
		case 0xD5:	push(r16::DE);	break;
		case 0xE5:	push(r16::HL);	break;

		case 0xF1:	pop(r16::AF);	break;
		case 0xC1:	pop(r16::BC);	break;
		case 0xD1:	pop(r16::DE);	break;
		case 0xE1:	pop(r16::HL);	break;


		/* 8bit ALU */
		case 0x87:	add(r8::A, r8::A);		break;
		case 0x80:	add(r8::A, r8::B);		break;
		case 0x81:	add(r8::A, r8::C);		break;
		case 0x82:	add(r8::A, r8::D);		break;
		case 0x83:	add(r8::A, r8::E);		break;
		case 0x84:	add(r8::A, r8::H);		break;
		case 0x85:	add(r8::A, r8::L);		break;
		case 0x86:	add(r8::A, r16::HL);	break;
		case 0xC6:	add(r8::A, read_pc8());	break;

		case 0x97:	sub(r8::A, r8::A);		break;
		case 0x90:	sub(r8::A, r8::B);		break;
		case 0x91:	sub(r8::A, r8::C);		break;
		case 0x92:	sub(r8::A, r8::D);		break;
		case 0x93:	sub(r8::A, r8::E);		break;
		case 0x94:	sub(r8::A, r8::H);		break;
		case 0x95:	sub(r8::A, r8::L);		break;
		case 0x96:	sub(r8::A, r16::HL);	break;
		case 0xD6:	sub(r8::A, read_pc8());	break;

		case 0xA7:	band(r8::A, r8::A);		break;
		case 0xA0:	band(r8::A, r8::B);		break;
		case 0xA1:	band(r8::A, r8::C);		break;
		case 0xA2:	band(r8::A, r8::D);		break;
		case 0xA3:	band(r8::A, r8::E);		break;
		case 0xA4:	band(r8::A, r8::H);		break;
		case 0xA5:	band(r8::A, r8::L);		break;
		case 0xA6:	band(r8::A, r16::HL);	break;
		case 0xE6:	band(r8::A, read_pc8());break;

		case 0xB7:	bor(r8::A, r8::A);		break;
		case 0xB0:	bor(r8::A, r8::B);		break;
		case 0xB1:	bor(r8::A, r8::C);		break;
		case 0xB2:	bor(r8::A, r8::D);		break;
		case 0xB3:	bor(r8::A, r8::E);		break;
		case 0xB4:	bor(r8::A, r8::H);		break;
		case 0xB5:	bor(r8::A, r8::L);		break;
		case 0xB6:	bor(r8::A, r16::HL);	break;
		case 0xF6:	bor(r8::A, read_pc8());	break;

		case 0xAF:	bxor(r8::A, r8::A);		break;
		case 0xA8:	bxor(r8::A, r8::B);		break;
		case 0xA9:	bxor(r8::A, r8::C);		break;
		case 0xAA:	bxor(r8::A, r8::D);		break;
		case 0xAB:	bxor(r8::A, r8::E);		break;
		case 0xAC:	bxor(r8::A, r8::H);		break;
		case 0xAD:	bxor(r8::A, r8::L);		break;
		case 0xAE:	bxor(r8::A, r16::HL);	break;
		case 0xEE:	bxor(r8::A, read_pc8());break;

		case 0xBF:	cp(r8::A, r8::A);		break;
		case 0xB8:	cp(r8::A, r8::B);		break;
		case 0xB9:	cp(r8::A, r8::C);		break;
		case 0xBA:	cp(r8::A, r8::D);		break;
		case 0xBB:	cp(r8::A, r8::E);		break;
		case 0xBC:	cp(r8::A, r8::H);		break;
		case 0xBD:	cp(r8::A, r8::L);		break;
		case 0xBE:	cp(r8::A, r16::HL);		break;
		case 0xFE:	cp(r8::A, read_pc8());	break;

		case 0x3C:	inc(r8::A);		break;
		case 0x04:	inc(r8::B);		break;
		case 0x0C:	inc(r8::C);		break;
		case 0x14:	inc(r8::D);		break;
		case 0x1C:	inc(r8::E);		break;
		case 0x24:	inc(r8::H);		break;
		case 0x2C:	inc(r8::L);		break;
		case 0x34:	incp(r16::HL);	break;

		case 0x3D:	dec(r8::A);		break;
		case 0x05:	dec(r8::B);		break;
		case 0x0D:	dec(r8::C);		break;
		case 0x15:	dec(r8::D);		break;
		case 0x1D:	dec(r8::E);		break;
		case 0x25:	dec(r8::H);		break;
		case 0x2D:	dec(r8::L);		break;
		case 0x35:	decp(r16::HL);	break;

		//16 bits ALU

		case 0x09:	add(r16::HL, r16::BC);	break;
		case 0x19:	add(r16::HL, r16::DE);	break;
		case 0x29:	add(r16::HL, r16::HL);	break;
		case 0x39:	add(r16::HL, r16::SP);	break;

		case 0xE8:	add(r16::SP, read_pc8());	break;

		case 0x03:	inc(r16::BC);	break;
		case 0x13:	inc(r16::DE);	break;
		case 0x23:	inc(r16::HL);	break;
		case 0x33:	inc(r16::SP);	break;

		case 0x0B:	dec(r16::BC);	break;
		case 0x1B:	dec(r16::DE);	break;
		case 0x2B:	dec(r16::HL);	break;
		case 0x3B:	dec(r16::SP);	break;

		// Misc

		case 0x27:	daa();	break;
		case 0x2F:	cpl();	break;
		case 0x3F:	ccf();	break;
		case 0x37:	scf();	break;
		case 0x00:	/*NOP*/	break;
		case 0x76:	halt();	break;
		case 0xF3:	di();	break;
		case 0xFB:	ei();	break;
		//TODO : case 10 00 => STOP

		// Roates and shift

		case 0x07:	rlca();	break;
		case 0x17:	rla();	break;
		case 0x0F:	rrca();	break;
		case 0x1F:	rra();	break;



		case 0xCB:
			std::invoke(extended_set[read_pc8()], *this);
			break;


		case 0xC3:
			*pc = read_pc16();
			break;
		default:
			break;
	}
}
