#include "emulator.h"
#include "debug_ui.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <chrono>


Emulator::Emulator(/* args */)
:cpu(&memory), gpu(&memory)
{

}

Emulator::~Emulator()
{
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

		return true;
	} else {
		return false;
	}
}

void Emulator::start()
{

}

void Emulator::step(uint8_t keys)
{
	const uint16_t CYCLES_BY_FRAME = 17556;
	uint16_t cycles_total = 0;

	if(!options.pause)
	{
		while(cycles_total < CYCLES_BY_FRAME)
		{
			// std::chrono::duration<double, std::milli> t_cpu, t_gpu, t_total;
			// auto total_start = std::chrono::steady_clock::now();
			// auto start = total_start;
			memory.update_joypad(keys);
			uint8_t cycles = cpu.step();

			if(memory.read_8bits(0x8001) == 0x39)
				options.pause = true;

			// auto end = std::chrono::steady_clock::now();
			// t_cpu = end - start;

			// start = std::chrono::steady_clock::now();

			gpu.step(cycles);
			cycles_total += cycles;

			// end = std::chrono::steady_clock::now();
			// t_gpu = end - start;

			// auto total_end = std::chrono::steady_clock::now();
			// t_total = total_end - total_start;

			// std::cout << " cpu time " << t_cpu.count() << " ms, \t" << (int)cycles << " cycles" << "\n";
			// std::cout << " gpu time " << t_gpu.count() << " ms" << "\n";

			// std::cout << "total time " <<  t_total.count() << " ms " << std::endl;
		}
	}

}

const uint32_t* Emulator::get_pixel_data()
{
	return gpu.get_pixel_data();
}

const GPU& Emulator::get_gpu_ref()
{
	return gpu;
}