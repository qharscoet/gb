#include "emulator.h"
#include <cstring>
#include <iostream>
#include <fstream>

Emulator::Emulator(/* args */)
{
	cpu = CPU(&memory);
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
		memory.load_content(file);
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