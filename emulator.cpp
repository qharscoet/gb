#include "emulator.h"
#include <cstring>
#include <iostream>
#include <fstream>

Emulator::Emulator(/* args */)
{
	std::memset(mmap, 0, 0x1000);
	cpu = CPU(mmap);
}

Emulator::~Emulator()
{
}

void Emulator::init()
{}

bool Emulator::load_rom(std::string filename)
{
	std::ifstream file;
	file.open(filename, std::ios::in | std::ios::binary);

	if(file.is_open()){
		file.read(mmap, 0x8000);
		file.close();

		return true;
	} else {
		return false;
	}
}

void Emulator::start()
{
	while(true){
		cpu.step();
	}
}