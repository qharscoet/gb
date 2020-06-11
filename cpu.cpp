#include "cpu.h"
#include <cstdint>

CPU::CPU(/* args */)
:pc(0x100), sp(0xFFFF)
{
}

CPU::CPU(char* memory)
:pc(0x100), sp(0xFFFF)
{
	this->memory = memory;

	write_r8_register(r8_name::H, 1);
	write_r8_register(r8_name::L, 2);

	uint16_t mabite = read_r16_register(r16_name::HL);
	int a = 4;
}

CPU::~CPU()
{
}

void CPU::write_r8_register(r8_name r, uint8_t value)
{
	registers[static_cast<uint8_t>(r)] = value;
}

uint8_t CPU::read_r8_register(r8_name r)
{
	return registers[static_cast<uint8_t>(r)];
}

uint16_t CPU::read_r16_register(r16_name r)
{
	return ((uint16_t*)registers)[static_cast<uint8_t>(r)];
}

uint8_t CPU::read_8_bit_from_memory(uint16_t addr)
{
	return memory[addr];
}

uint16_t CPU::read_16_bit_from_memory(uint16_t addr)
{
	uint8_t lsb = memory[addr++];
	uint8_t msb = memory[addr++];

	return (uint16_t)(msb << 8) | (uint16_t)(lsb);
}

void CPU::write_8_bits_to_memory(uint16_t addr, uint8_t value)
{
	memory[addr] = value;
}

void CPU::ld(r8_name r, uint8_t value)
{
	write_r8_register(r, value);
}

void CPU::ld(r8_name r1, r8_name r2)
{
	// registers[static_cast<uint8_t>(r1)] = registers[static_cast<uint8_t>(r2)];
	write_r8_register(r1, read_r8_register(r2));
}


void CPU::ld(r8_name r1, r16_name r2)
{
	write_r8_register(r1, read_8_bit_from_memory(read_r16_register(r2)));
	//registers[static_cast<uint8_t>(r1)] =  memory[(uint16_t)registers[static_cast<uint8_t>(r2)]];
}

void CPU::ld(r16_name r1, r8_name r2)
{
	write_8_bits_to_memory(read_r16_register(r1), read_r8_register(r2));
	//memory[(uint16_t)registers[static_cast<uint8_t>(r1)]] = registers[static_cast<uint8_t>(r2)];
}


void CPU::step()
{
	uint8_t opcode = memory[pc++];

	//handle instruction
	switch(opcode) {
		case 0x06: ld(r8_name::B, read_8_bit_from_memory(pc++)); break;
		case 0x0E: ld(r8_name::C, read_8_bit_from_memory(pc++)); break;
		case 0x16: ld(r8_name::D, read_8_bit_from_memory(pc++)); break;
		case 0x1E: ld(r8_name::E, read_8_bit_from_memory(pc++)); break;
		case 0x26: ld(r8_name::H, read_8_bit_from_memory(pc++)); break;
		case 0x2E: ld(r8_name::L, read_8_bit_from_memory(pc++)); break;
		case 0x40: ld(r8_name::B,r8_name::B);	break;
		case 0x41: ld(r8_name::B,r8_name::C);	break;
		case 0x42: ld(r8_name::B,r8_name::D);	break;
		case 0x43: ld(r8_name::B,r8_name::E);	break;
		case 0x44: ld(r8_name::B,r8_name::H);	break;
		case 0x45: ld(r8_name::B,r8_name::L);	break;
		case 0x46: ld(r8_name::B,r16_name::HL);	break;
		case 0x47: ld(r8_name::B,r8_name::A);	break;
		case 0x48: ld(r8_name::C,r8_name::B);	break;
		case 0x49: ld(r8_name::C,r8_name::C);	break;
		case 0x4A: ld(r8_name::C,r8_name::D);	break;
		case 0x4B: ld(r8_name::C,r8_name::E);	break;
		case 0x4C: ld(r8_name::C,r8_name::H);	break;
		case 0x4D: ld(r8_name::C,r8_name::L);	break;
		case 0x4E: ld(r8_name::C,r16_name::HL);	break;
		case 0x4F: ld(r8_name::C,r8_name::A);	break;
		case 0x50: ld(r8_name::D,r8_name::B);	break;
		case 0x51: ld(r8_name::D,r8_name::C);	break;
		case 0x52: ld(r8_name::D,r8_name::D);	break;
		case 0x53: ld(r8_name::D,r8_name::E);	break;
		case 0x54: ld(r8_name::D,r8_name::H);	break;
		case 0x55: ld(r8_name::D,r8_name::L);	break;
		case 0x56: ld(r8_name::D,r16_name::HL);	break;
		case 0x57: ld(r8_name::D,r8_name::A);	break;
		case 0x58: ld(r8_name::E,r8_name::B);	break;
		case 0x59: ld(r8_name::E,r8_name::C);	break;
		case 0x5A: ld(r8_name::E,r8_name::D);	break;
		case 0x5B: ld(r8_name::E,r8_name::E);	break;
		case 0x5C: ld(r8_name::E,r8_name::H);	break;
		case 0x5D: ld(r8_name::E,r8_name::L);	break;
		case 0x5E: ld(r8_name::E,r16_name::HL);	break;
		case 0x5F: ld(r8_name::E,r8_name::A);	break;
		case 0x60: ld(r8_name::H,r8_name::B);	break;
		case 0x61: ld(r8_name::H,r8_name::C);	break;
		case 0x62: ld(r8_name::H,r8_name::D);	break;
		case 0x63: ld(r8_name::H,r8_name::E);	break;
		case 0x64: ld(r8_name::H,r8_name::H);	break;
		case 0x65: ld(r8_name::H,r8_name::L);	break;
		case 0x66: ld(r8_name::H,r16_name::HL);	break;
		case 0x67: ld(r8_name::H,r8_name::A);	break;
		case 0x68: ld(r8_name::L,r8_name::B);	break;
		case 0x69: ld(r8_name::L,r8_name::C);	break;
		case 0x6A: ld(r8_name::L,r8_name::D);	break;
		case 0x6B: ld(r8_name::L,r8_name::E);	break;
		case 0x6C: ld(r8_name::L,r8_name::H);	break;
		case 0x6D: ld(r8_name::L,r8_name::L);	break;
		case 0x6E: ld(r8_name::L,r16_name::HL);	break;
		case 0x6F: ld(r8_name::L,r8_name::A);	break;
		case 0x70: ld(r16_name::HL,r8_name::B);	break;
		case 0x71: ld(r16_name::HL,r8_name::C);	break;
		case 0x72: ld(r16_name::HL,r8_name::D);	break;
		case 0x73: ld(r16_name::HL,r8_name::E);	break;
		case 0x74: ld(r16_name::HL,r8_name::H);	break;
		case 0x75: ld(r16_name::HL,r8_name::L);	break;
		case 0x77: ld(r16_name::HL,r8_name::A);	break;
		case 0x78: ld(r8_name::A,r8_name::B);	break;
		case 0x79: ld(r8_name::A,r8_name::C);	break;
		case 0x7A: ld(r8_name::A,r8_name::D);	break;
		case 0x7B: ld(r8_name::A,r8_name::E);	break;
		case 0x7C: ld(r8_name::A,r8_name::H);	break;
		case 0x7D: ld(r8_name::A,r8_name::L);	break;
		case 0x7E: ld(r8_name::A,r16_name::HL);	break;
		case 0x7F: ld(r8_name::A,r8_name::A);	break;

		case 0xC3:
			pc = read_16_bit_from_memory(pc);
			break;
		default:
			break;
	}
}
