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

	std::vector<uint8_t> rom;
	std::vector<uint8_t> ram;

protected:
	mbc_type type;
	uint16_t current_rom;
	uint8_t current_ram;

	bool ram_enabled;

public:

	MBC() = default;
	MBC(mbc_type type, uint32_t romsize, uint32_t ramsize, std::istream &file);
	virtual ~MBC() = default;

	void reset();
	virtual void write(uint16_t addr, uint8_t value);
	virtual void write_ram(uint16_t addr, uint8_t value);
	void write_ram(uint16_t addr, uint16_t value);

	uint8_t read_rom(uint16_t addr) const;
	virtual uint8_t read_ram(uint16_t addr) const;

	size_t rom_banks_count() const;
	size_t ram_banks_count() const;

	virtual void rtc_add_second();
	virtual void set_rtc(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds);

	bool use_ram() const;
	virtual void dump_ram(std::ostream &file) const;
	virtual void load_ram(std::istream &file);

	uint8_t* get_rom_data(uint16_t addr);
	uint8_t* get_ram_data(uint16_t addr);
};


class MBC1 : public MBC
{
	private:
		bool rom_ram_mode;
	public:
		MBC1(mbc_type type, uint32_t romsize, uint32_t ramsize, std::istream &file);
		void write(uint16_t addr, uint8_t value);
};

class MBC3 : public MBC
{
	private:
		uint8_t RTC_reg[5]; //Clock registers;
		uint8_t latched_reg[5];
		bool is_latched;
	public:
		MBC3(mbc_type type, uint32_t romsize, uint32_t ramsize, std::istream& file);

		void rtc_add_second();
		virtual void set_rtc(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds);

		void write(uint16_t addr, uint8_t value);

		void write_ram(uint16_t addr, uint8_t value);
		uint8_t read_ram(uint16_t addr) const;

		/* We need to handle the RTC registers in the save file and save timestamp
			to simulate the passage of time while the console is off */
		void dump_ram(std::ostream &file) const;
		void load_ram(std::istream &file);
};

class MBC5 : public MBC
{
	public:
		MBC5(mbc_type type, uint32_t romsize, uint32_t ramsize, std::istream &file);
		void write(uint16_t addr, uint8_t value);
};

#endif
