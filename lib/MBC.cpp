#include "MBC.h"

#include <algorithm>
#include <ctime>
#include <cstring>

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

void MBC::rtc_add_second()
{
	//Do nothing
}

void MBC::set_rtc(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds)
{}

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
		current_rom &= rom_banks_count() - 1;
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
	:MBC(type, romsize, ramsize, file), is_latched(false)
{
	memset(RTC_reg, 0, 5);
	memset(latched_reg, 0, 5);
}

void MBC3::rtc_add_second()
{
	enum { RTC_S = 0, RTC_M, RTC_H, RTC_DL, RTC_DH};
	if(RTC_reg[RTC_S]++ == 60) // Seconds
	{
		RTC_reg[RTC_S] = 0;
		if(RTC_reg[RTC_M]++ == 60) //Minutes
		{
			RTC_reg[RTC_M] = 0;
			if (RTC_reg[RTC_H]++ == 24) //Hours
			{
				RTC_reg[RTC_H] = 0;

				if(RTC_reg[RTC_DL]++ == 0xFF) // 8 lower bits overflow
				{
					RTC_reg[RTC_DH]++;
					//If last bit is there we have overflowed and we set carry bit
					if((RTC_reg[RTC_DH] & 0x1) == 0)
					{
						RTC_reg[RTC_DH] |= (1 << 7);
					}

					RTC_reg[RTC_DH] &= 0xC1;
				}
			}
		}
	}
}

void MBC3::set_rtc(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	enum { RTC_S = 0, RTC_M, RTC_H, RTC_DL, RTC_DH};
	if(seconds < 60) RTC_reg[RTC_S] = seconds;
	if(minutes < 60) RTC_reg[RTC_M] = minutes;
	if(hours < 24) RTC_reg[RTC_H] = hours;

	if(days < 512) {
		RTC_reg[RTC_DL] = days & 0xFF;
		RTC_reg[RTC_DH] |= days >> 8;
	}
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
		if(!is_latched && value == 0x01)
		{
			is_latched = true;
			memcpy(latched_reg, RTC_reg, 5);
		} else if(value == 0x00)
		{
			is_latched = false;
		}
	}
}

void MBC3::write_ram(uint16_t addr, uint8_t value)
{
	if (current_ram >= 0x08 && current_ram <= 0x0C)
		RTC_reg[current_ram - 0x08] = value;
	else if (current_ram <= 0x07)
		MBC::write_ram(addr, value);
}
uint8_t MBC3::read_ram(uint16_t addr) const
{
	if (current_ram >= 0x08 && current_ram <= 0x0C)
		return is_latched ? latched_reg[current_ram - 0x08] : RTC_reg[current_ram - 0x08];
	else if (current_ram <= 0x07)
		return MBC::read_ram(addr);
	else
		return 0;
}

void MBC3::dump_ram(std::ostream &file) const
{
	MBC::dump_ram(file);
	file.write((char*)(&RTC_reg[0]), 5);
	std::time_t now = std::time(nullptr);

	file.write((char*)&now, sizeof(std::time_t));
}

void MBC3::load_ram(std::istream &file)
{
	MBC::load_ram(file);
	file.read((char*)(&RTC_reg[0]), 5);

	std::time_t timestamp;
	file.read((char*)&timestamp, sizeof(std::time_t));

	//TODO : refactor this loop ?
	std::time_t now = std::time(nullptr);
	for(int i = 0; i < now - timestamp; i++)
	{
		rtc_add_second();
	}
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