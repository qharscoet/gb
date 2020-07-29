#include <cstdint>
#include <cstring>

#include <fstream>

#define MEMSIZE 0x10000

class Memory
{
private:
	char mmap[MEMSIZE];

	void DMATransfer(uint8_t src);

public:
	Memory(/* args */);
	~Memory();

	void load_content(std::istream &file);

	uint8_t read_8bits(uint16_t addr);
	uint16_t read_16bits(uint16_t addr);

	void write_8bits(uint16_t addr, uint8_t value);
	void write_16bits(uint16_t addr, uint16_t value);
};
