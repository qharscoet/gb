#ifndef _EMULATOR_H_
#define _EMULATOR_H_

#include "cpu.h"
#include "gpu.h"

#include <string>

class Emulator
{

	friend class Debug_Display;

private:

	enum class emu_state : uint8_t {
		IDLE = 0,
		RUNNING,
		NEED_RELOAD,
		QUIT,
	};

	emu_state state;

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
	void quit();
	void reset();
	void stop();

	const uint32_t* get_pixel_data() const;
	const std::string get_game_name() const;

	void save() const;
	void load_save();

	//state functions
	bool is_running();
	bool is_exiting();
	bool needs_reload();
};

#endif