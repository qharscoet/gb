#include "MBC.h"

#include <algorithm>

MBC::MBC(mbc_type type, uint32_t romsize, uint32_t ramsize, std::istream &file)
{
	this->type = type;

	rom.resize(romsize);
	ram.resize(ramsize);

	file.seekg(0);
	file.read((char*)(&rom[0]), romsize);

	current_rom = 1;
	current_ram = 0;

	ram_enabled = false;
}

void MBC::reset()
{
	if(rom.size() > 0)
		std::fill(rom.begin(), rom.end(), 0);

	if(ram.size() > 0)
		std::fill(ram.begin(), ram.end(), 0);
}

void MBC::write_ram(uint16_t addr, uint8_t value)
{
	if(ram_enabled)
		ram[current_ram * 0x2000 + addr] = value;
}

uint8_t MBC::read_rom(uint16_t addr) const
{
	return rom[current_rom * 0x4000 + addr];
}
uint8_t MBC::read_ram(uint16_t addr) const
{
	return ram_enabled ? ram[current_ram * 0x2000 + addr] : 0;
}

size_t MBC::rom_banks_count() const
{
	//return rom.size()/0x4000;
	return rom.size() >> 14;
}
size_t MBC::ram_banks_count() const
{
	//return ram.size()/0x2000;
	return ram.size() >> 13;
}

bool MBC::use_ram() const
{
	return ram.size() > 0;
}

void MBC::dump_ram(std::ostream &file) const
{
	file.write((char*)(&ram[0]), ram.size());
}

void MBC::load_ram(std::istream &file)
{
	file.read((char*)(&ram[0]), ram.size());
}

void MBC::write(uint16_t addr, uint8_t value)
{}

MBC1::MBC1(mbc_type type, uint32_t romsize, uint32_t ramsize, std::istream &file)
:MBC(type, romsize, ramsize, file)
{
	rom_ram_mode = false;
}

void MBC1::write(uint16_t addr, uint8_t value)
{
	// external RAM enable
	if (addr < 0x2000)
	{
		ram_enabled = ((value & 0x0F) == 0xA);
	}
	else if (addr < 0x4000)
	{
		if (value == 0)
			value = 1;

		current_rom = (current_rom & 0xE0) | (value & 0x1F);
	}
	else if (addr < 0x6000)
	{
		value &= 0x03;
		if (rom_ram_mode)
			current_ram = value;
		else
		{
			current_rom = (value << 5) | (current_rom & 0x1F);
			current_rom &= rom_banks_count() -1;
		}
	}
	else
	{
		rom_ram_mode = value;
	}
}

MBC3::MBC3(mbc_type type, uint32_t romsize, uint32_t ramsize, std::istream& file)
	:MBC(type, romsize, ramsize, file)
{
	memset(RTC_reg, 0, 5);
}

void MBC3::write(uint16_t addr, uint8_t value)
{
	// external RAM enable
	if (addr < 0x2000)
	{
		ram_enabled = ((value & 0x0F) == 0xA);
	}
	else if (addr < 0x4000)
	{
		if (value == 0)
			value = 1;

		current_rom = value & 0x7F;
	}
	else if (addr < 0x6000)
	{
		current_ram = value;
	}
	else
	{
		//TODO: handle latch
	}
}

void MBC3::write_ram(uint16_t addr, uint8_t value)
{
	if (current_ram >= 0x08 && current_ram <= 0x0C)
		RTC_reg[addr - 0x08] = value;
	else if (current_ram <= 0x03)
		MBC::write_ram(addr, value);
}
uint8_t MBC3::read_ram(uint16_t addr) const
{
	if (current_ram >= 0x08 && current_ram <= 0x0C)
		return RTC_reg[addr - 0x08];
	else if (current_ram <= 0x03)
		return MBC::read_ram(addr);
}

MBC5::MBC5(mbc_type type, uint32_t romsize, uint32_t ramsize, std::istream &file)
:MBC(type, romsize, ramsize, file)
{}

void MBC5::write(uint16_t addr, uint8_t value)
{
	if (addr < 0x2000)
	{
		ram_enabled = (value == 0xA);
	} else if( addr < 0x3000) {
		current_rom = (current_rom & 0xFF00) | value;
	} else if (addr < 0x4000) {
		current_rom = (current_rom & 0xFEFF) | ((value & 0x1) << 8);
	} else if (addr < 0x5000) {
		current_ram = (current_ram & 0xF0) | (value & 0x0F);
	}
}

uint8_t* MBC::get_rom_data(uint16_t addr)
{
	return &rom[current_rom * 0x4000 + addr];
}

uint8_t* MBC::get_ram_data(uint16_t addr)
{
	return &ram[current_ram * 0x2000 + addr];
}