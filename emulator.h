#include "cpu.h"
#include "gpu.h"


#include <string>

class Emulator
{
private:
	Memory memory; //64Ko memory map
	CPU cpu;
	GPU gpu;
public:
	Emulator(/* args */);
	~Emulator();

	void init();
	bool load_rom(std::string filename);
	void start();
	void step(uint8_t inputs);

	uint32_t* get_pixel_data();
};