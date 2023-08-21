#include "emulator.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <chrono>
#include <filesystem>

EMULATOR_API emu_options options;

Emulator::Emulator()
:cpu(&memory), gpu(&memory), apu(memory.get_data_span<0x30>(0xFF10)), state(emu_state::IDLE)
{
	memory.set_apu(&apu);
	frame_count = 0;
	serial_byte = 0;
	serial_stop = false;
}

Emulator::~Emulator()
{
	save();
	if (serial_thread.joinable()){
		serial_stop = true;
		serial_thread.join();
	}
	this->close_network();

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
		if (memory.cgb_enabled())
		{
			memory.set_palette(gpu.get_palette_data());
		}
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

	if (serial_thread.joinable())
	{
		serial_stop = true;
		serial_thread.join();
	}
	serial_stop = false;
	serial_thread = std::thread(&Emulator::serial_run, this);

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
			uint8_t cycles = cpu.step() *4;

			uint8_t speed_multiplier = cpu.is_double_speed() + 1;
			cycles /= speed_multiplier;
			if(cycles == 0)
				cycles = 1;

			// step_serial();
			gpu.step(cycles);
			apu.step(cycles);
			// cycles_total += cycles;

	// 	}
	// }

	return cycles;
}

void Emulator::serial_run()
{
	while(!serial_stop)
	{
			step_serial();
	}
}

void Emulator::step_serial()
{
	static const uint16_t SB = 0xFF01;
	static const uint16_t SC = 0xFF02;

	uint8_t serial_control = memory.read_8bits(SC);
	if (serial_control & (1 << 7))
	{
		if (serial_control & 1)
		{
			if(send_byte(memory.read_8bits(SB), true))
			{
				// receive result as both are done at the same time
				receive_byte(&serial_byte, true);
				memory.write_8bits(SB, serial_byte);
				serial_byte = 0;
				serial_control &= ~(1 << 7); //Reset bit 7
				memory.write_8bits(SC, serial_control);
				memory.request_interrupt(Memory::interrupt_id::IO);
			}
		}
		else {
			//If we received something we are probably the slave
			if (receive_byte(&serial_byte, false))
			{
				//send our content back
				send_byte(memory.read_8bits(SB), true);
				memory.write_8bits(SB, serial_byte);
				serial_byte = 0;
				serial_control &= ~(1 << 7); //Reset bit 7
				memory.write_8bits(SC, serial_control);
				memory.request_interrupt(Memory::interrupt_id::IO);
			}
		}
	}
}

void Emulator::update_rtc()
{
	if(frame_count++ >= 60){
		memory.update_rtc();
		frame_count = 0;
	}
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

const bool Emulator::is_gameboy_color() const
{
	return memory.cgb_enabled();
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
void Emulator::listen_network(const char* port)
{
	init_network();
	init_listen_socket(port);
}
void Emulator::connect_network(const char* addr, const char* port)
{
	init_network();
	init_connect_socket(addr, port);
}

void Emulator::close_network()
{
	close_socket();
}

enum network_state Emulator::is_connected()
{
	return get_network_state();
}

bool Emulator::send_byte(const uint8_t byte, bool blocking)
{
	if(is_connected())
		return send_data((char*)&byte, 1, blocking);

	return 0;
}
bool Emulator::receive_byte(uint8_t *byte, bool blocking)
{
	if(is_connected())
		return receive_data((char*)byte, 1, blocking);

	return 0;
}

//Memory
uint8_t Emulator::read_8bits(uint16_t addr) const
{ return memory.read_8bits(addr); }
uint8_t *const Emulator::get_data(uint16_t addr) const
{ return memory.get_data(addr); }
uint8_t *const Emulator::get_vram_data(uint16_t addr, int bank) const
{ return memory.get_vram_data(addr, bank); }
void Emulator::set_rtc(uint16_t days, uint8_t hours, uint8_t minutes, uint8_t seconds)
{ memory.set_rtc(days,hours, minutes, seconds); }

//CPU
uint16_t Emulator::get_pc() const
{return cpu.get_pc();}

//GPU
void Emulator::draw_full_bg(uint32_t *pixels) const
{ gpu.draw_full_bg(pixels); }
void Emulator::draw_bg_tiles(uint32_t *pixels, bool bank) const
{ gpu.display_bg_tiles(pixels, bank); }
uint32_t Emulator::get_palette_color(bool bg, uint8_t palette, uint8_t col_id) const
{ return gpu.get_palette_color(bg, palette, col_id); }