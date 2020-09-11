#include "emulator.h"
#include "options.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <chrono>

extern emu_options options;

Emulator::Emulator()
:cpu(&memory), gpu(&memory), state(emu_state::IDLE)
{

}

Emulator::~Emulator()
{
	save();
}

void Emulator::init()
{
	options.pause = false;
}

bool Emulator::load_rom(std::string filename)
{
	std::ifstream file;
	file.open(filename, std::ios::in | std::ios::binary);

	if(file.is_open()){
		memory.load_content(file);
		file.close();

		load_save();

		return true;
	} else {
		return false;
	}
}

void Emulator::start()
{
	cpu.reset();
	cpu.init();
	state = emu_state::RUNNING;
}

void Emulator::step(uint8_t keys)
{
	const uint16_t CYCLES_BY_FRAME = 17556;
	uint16_t cycles_total = 0;

	if(!options.pause)
	{
		while(cycles_total < CYCLES_BY_FRAME)
		{
			memory.update_joypad(keys);
			uint8_t cycles = cpu.step();

			gpu.step(cycles);
			cycles_total += cycles;
		}
	}

}

const uint32_t* Emulator::get_pixel_data() const
{
	return gpu.get_pixel_data();
}

const std::string Emulator::get_game_name() const
{
	return memory.get_data(0x0134);
}

void Emulator::save() const
{
	std::ofstream file;
	file.open(get_game_name() + ".sav", std::ios::out | std::ios::binary | std::ios::trunc);
	memory.dump_ram(file);
	file.close();
}

void Emulator::load_save()
{
	std::ifstream file;
	file.open(get_game_name() + ".sav", std::ios::in | std::ios::binary);

	if(file.is_open())
	{
		memory.load_ram(file);
		file.close();
	}
}

bool Emulator::is_running()
{
	return state == emu_state::RUNNING;
}
bool Emulator::is_exiting()
{
	return state == emu_state::QUIT;
}
bool Emulator::needs_reload()
{
	return state == emu_state::NEED_RELOAD;
}

void Emulator::quit()
{
	state = emu_state::QUIT;
}

void Emulator::reset()
{
	state = emu_state::NEED_RELOAD;
}

void Emulator::stop()
{
	state = emu_state::IDLE;
}