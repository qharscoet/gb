#include <cstdlib>
#include <iostream>

#include "emulator.h"
#include "display.h"


int main(int argc, char const *argv[])
{
	Emulator emu;
	Display display;
	emu.init();
	display.init();

	if(argc > 1) {
		if(!emu.load_rom(argv[1]))
		{
			std::cout << "Error opening file " << argv[1] << std::endl;
		}
		else
		{
			//emu.start();
			while(display.handle_events())
			{
				emu.step();
				display.update(emu.get_pixel_data());
				display.render();
			}
		}

	} else {
		std::cout << "please indicate rom file" << std::endl;
	}

	std::cout << "COUCOU" << std::endl;

	display.free();
	return 0;
}
