#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <cstdint>
#include <cstring>

#include <fstream>

#define MEMSIZE 0x10000

class Memory
{
private:
	char mmap[MEMSIZE];
	uint8_t joypad_keys;

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

	Memory(/* args */);
	~Memory();

	void load_content(std::istream &file);
	void load_content(const uint8_t* data, uint32_t size);

	void update_joypad(uint8_t keys);

	uint8_t read_8bits(uint16_t addr);
	uint16_t read_16bits(uint16_t addr);

	void write_8bits(uint16_t addr, uint8_t value);
	void write_16bits(uint16_t addr, uint16_t value);

	void request_interrupt(interrupt_id id);
};

#endif