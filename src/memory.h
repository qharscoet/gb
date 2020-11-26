#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <cstdint>
#include <cstring>

#include <fstream>

#include <span>

#include "MBC.h"
#include "sound.h"

#define MEMSIZE 0x10000
#define VRAM_BANK_SIZE 0x2000
#define WRAM_BANK_SIZE 0x1000

union palette
{
	union color{
		uint8_t hl[2];
		uint16_t value;
	} palette[8][4];
	uint8_t array[8][4][2];
};

typedef palette PaletteData;

class Memory
{
private:
	// TODO : change mmap to not allocate areas managed by other components (MBC and audio atm)
	uint8_t* mmap;

	bool is_cgb;
	uint8_t* vram_banks;
	uint8_t* wram_banks;

	uint8_t joypad_keys;

	MBC* mbc;
	Sound* apu;

	PaletteData* cgb_palette_data;


	void DMATransfer(uint8_t src);
	void HDMATransfer(uint8_t length_mode);

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
	void set_palette(PaletteData* palette);

	void reset();

	void load_content(std::istream &file);
	void load_content(const uint8_t* data, uint32_t size);

	void update_joypad(uint8_t keys);

	uint8_t read_8bits(uint16_t addr) const;
	uint16_t read_16bits(uint16_t addr) const;
	uint8_t read_vram(uint16_t addr, bool bank) const;

	void write_8bits(uint16_t addr, uint8_t value);
	void write_16bits(uint16_t addr, uint16_t value);

	void request_interrupt(interrupt_id id);

	uint8_t* const get_data(uint16_t addr) const;

	template <size_t size>
	std::span<uint8_t, size> get_data_span(uint16_t addr) const;

	void dump_ram(std::ostream &file) const;
	void load_ram(std::istream &file);

	bool use_external_ram() const;
	bool use_mbc() const;
	bool cgb_enabled() const;
};

template <size_t size>
std::span<uint8_t, size> Memory::get_data_span(uint16_t addr) const
{
	return std::span<uint8_t, size>(&mmap[addr], size);
}

#endif