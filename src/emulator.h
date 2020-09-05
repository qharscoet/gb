#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "cpu.h"
#include "gpu.h"

#include <string>

class Emulator
{

	friend void debug_ui_render(Emulator &);

private:
	Memory memory; //64Ko memory map
	CPU cpu;
	GPU gpu;

public:
	Emulator();
	~Emulator();

	void init();
	bool load_rom(std::string filename);
	void start();
	void step(uint8_t inputs);

	const uint32_t* get_pixel_data() const;
	const std::string get_game_name() const;
	void save() const;
	void load_save();
};

#endif