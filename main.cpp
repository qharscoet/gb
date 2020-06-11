#include <cstdlib>
#include <iostream>

#include "emulator.h"


int main(int argc, char const *argv[])
{
	Emulator emu;
	emu.init();

	if(argc > 1) {
		if(!emu.load_rom(argv[1]))
			std::cout << "Error opening file " << argv[1] << std::endl;
	} else {
		std::cout << "please indicate rom file" << std::endl;
	}

	std::cout << "COUCOU" << std::endl;
	return 0;
}
