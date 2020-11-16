#include "emulator.h"
#include "options.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>

extern emu_options options;

Emulator::Emulator()
:cpu(&memory), gpu(&memory), apu(memory.get_data_span<0x2F>(0xFF10)), state(emu_state::IDLE)
{
	memory.set_apu(&apu);
}

Emulator::~Emulator()
{
	save();
}

void Emulator::init()
{
	options.pause = false;
}

bool Emulator::load_rom()
{
	return load_rom(rom_filename);
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

uint8_t Emulator::step(uint8_t keys)
{
	// const uint16_t CYCLES_BY_FRAME = 17556;
	// uint16_t cycles_total = 0;

	// if(!options.pause)
	// {
	// 	while(cycles_total < CYCLES_BY_FRAME)
	// 	{
			memory.update_joypad(keys);
			uint8_t cycles = cpu.step();

			gpu.step(cycles);
			apu.step(cycles);
			// cycles_total += cycles;

	// 	}
	// }

	return cycles;
}

const uint32_t* Emulator::get_pixel_data() const
{
	return gpu.get_pixel_data();
}

const std::string Emulator::get_game_name() const
{
	std::string str;
	if(rom_filename != "")
	{
		str = std::string((char*)(memory.get_data(0x0134)), 16);
		size_t pos = str.find('\0');
		if(pos != std::string::npos)
			str.resize(pos);
	} else
	{
		str = "GameBoy Emulator";
	}
	return str;
}

const float* Emulator::get_audio_data() const
{
	return apu.get_sound_data();
}

void Emulator::clear_audio()
{
	apu.clear_data();
}

const std::string Emulator::get_rom_dir() const
{
	return std::filesystem::path(rom_filename).remove_filename().string();
}

void Emulator::save() const
{
	if(memory.use_external_ram()){
		std::ofstream file;
		file.open(get_rom_dir() + get_game_name() + ".sav", std::ios::out | std::ios::binary | std::ios::trunc);
		if(file.is_open()){
			memory.dump_ram(file);
			file.close();
		}
	}
}

void Emulator::load_save()
{
	if (memory.use_external_ram())
	{
		std::ifstream file;
		file.open(get_rom_dir() + get_game_name() + ".sav", std::ios::in | std::ios::binary);

		if(file.is_open())
		{
			memory.load_ram(file);
			file.close();
		}
	}
}

void Emulator::set_rom_file(std::string filename)
{
	rom_filename = filename;
	//reset();
}

bool Emulator::is_running() const
{
	return state == emu_state::RUNNING;
}
bool Emulator::is_exiting() const
{
	return state == emu_state::QUIT;
}
bool Emulator::needs_reload() const
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