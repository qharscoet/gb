#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <cstdint>
#include <cstring>

#include <fstream>

#include <span>

#include "MBC.h"
#include "sound.h"

#define MEMSIZE 0x10000

class Memory
{
private:
	// TODO : change mmap to not allocate areas managed by other components (MBC and audio atm)
	char* mmap;
	uint8_t joypad_keys;

	MBC* mbc;
	Sound* apu;

	void DMATransfer(uint8_t src);

public:
	enum class interrupt_id : uint8_t
	{
		VBLANK = 0,
		STAT,
		TIMER,
		IO,
		JOYPAD
	};

	Memory();
	~Memory();

	void set_apu(Sound* apu);

	void reset();

	void load_content(std::istream &file);
	void load_content(const uint8_t* data, uint32_t size);

	void update_joypad(uint8_t keys);

	uint8_t read_8bits(uint16_t addr) const;
	uint16_t read_16bits(uint16_t addr) const;

	void write_8bits(uint16_t addr, uint8_t value);
	void write_16bits(uint16_t addr, uint16_t value);

	void request_interrupt(interrupt_id id);

	char* const get_data(uint16_t addr) const;

	template <size_t size>
	std::span<uint8_t, size> get_data_span(uint16_t addr) const;

	void dump_ram(std::ostream &file) const;
	void load_ram(std::istream &file);

	bool use_external_ram() const;
	bool use_mbc() const;
};

template <size_t size>
std::span<uint8_t, size> Memory::get_data_span(uint16_t addr) const
{
	return std::span<uint8_t, size>(&((uint8_t *)mmap)[addr], size);
}

#endif