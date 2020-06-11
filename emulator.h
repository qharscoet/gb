#include "cpu.h"

#include <string>

class Emulator
{
private:
	char mmap[0x10000]; //64Ko memory map
	CPU cpu;
public:
	Emulator(/* args */);
	~Emulator();

	void init();
	bool load_rom(std::string filename);
	void start();
};