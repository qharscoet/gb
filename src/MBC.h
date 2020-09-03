#ifndef __MBC_H__
#define __MBC_H__

#include <cstdint>
#include <vector>

#include <fstream>

class MBC
{

public:
enum class mbc_type : uint8_t
{
	ROM = 0,
	MBC1 = 1,
	MBC1_RAM = 2,
	MBC1_RAM_BATTERY = 3,
};

private:

	std::vector<char> rom;
	std::vector<char> ram;

protected:
	mbc_type type;
	uint8_t current_rom;
	uint8_t current_ram;

	bool ram_enabled;
	bool rom_ram_mode;

public:

	MBC() = default;
	MBC(mbc_type type, uint32_t romsize, uint32_t ramsize, std::istream &file);
	~MBC() = default;

	virtual void write(uint16_t addr, uint8_t value);
	void write_ram(uint16_t addr, uint8_t value);
	void write_ram(uint16_t addr, uint16_t value);

	uint8_t read_rom(uint16_t addr) const;
	uint8_t read_ram(uint16_t addr) const;

	uint8_t rom_banks_count() const;
	uint8_t ram_banks_count() const;

	bool use_ram() const;
};


class MBC1 : public MBC
{
	public:
		MBC1(mbc_type type, uint32_t romsize, uint32_t ramsize, std::istream &file);
		void write(uint16_t addr, uint8_t value);
};

class MBC5 : public MBC
{
	public:
		void write(uint16_t addr, uint8_t value);
};

#endif
