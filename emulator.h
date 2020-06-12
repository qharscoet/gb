#include "cpu.h"

#include <string>

class Emulator
{
private:
	Memory memory; //64Ko memory map
	CPU cpu;
	// GPU gpu;
public:
	Emulator(/* args */);
	~Emulator();

	void init();
	bool load_rom(std::string filename);
	void start();
};