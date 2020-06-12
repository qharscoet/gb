#include "cpu.h"
#include <cstdint>

CPU::CPU(/* args */)
:pc(0x100), sp(0xFFFF)
{
}

CPU::CPU(Memory* memory)
:pc(0x100), sp(0xFFFF)
{
	this->memory = memory;
}

CPU::~CPU()
{
}

void CPU::write_r8(r8 r, uint8_t value)
{
	registers[static_cast<uint8_t>(r)] = value;
}

uint8_t CPU::read_r8(r8 r)
{
	return registers[static_cast<uint8_t>(r)];
}

uint16_t CPU::read_r16(r16 r)
{
	return ((uint16_t*)registers)[static_cast<uint8_t>(r)];
}

uint8_t CPU::read_pc8()
{
	return memory->read_8bits(pc++);
}

uint16_t CPU::read_pc16()
{
	uint16_t ret = memory->read_16bits(pc);
	pc += 2;

	return ret;
}

void CPU::step()
{
	uint8_t opcode = memory->read_8bits(pc++);

	//handle instruction
	switch(opcode) {
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
		case 0x02: ld(r16::BC, r8::A);		break;
		case 0x12: ld(r16::DE, r8::A);		break;
		case 0xEA: ld(read_pc16(), r8::A);	break;


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

		case 0xC3:
			pc = memory->read_16bits(pc); pc+=2;
			break;
		default:
			break;
	}
}
