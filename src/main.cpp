#include <cstdlib>
#include <iostream>
#include <chrono>

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
				std::chrono::duration<double, std::milli> t_emu, t_display, t_total;
				auto total_start = std::chrono::steady_clock::now();
				auto start = total_start;

				emu.step(display.get_keystate());

				auto end = std::chrono::steady_clock::now();
				t_emu = end - start;

				start = std::chrono::steady_clock::now();

				display.update(emu.get_pixel_data(), emu.get_gpu_ref());
				//display.render();

				end = std::chrono::steady_clock::now();
				t_display = end - start;

				auto total_end = std::chrono::steady_clock::now();
				t_total = total_end - total_start;

				std::cout << " emu time " << t_emu.count() << " ms "<< "\n";
				std::cout << " display time " << t_display.count() << " ms" << "\n";

				std::cout << "total time " <<  t_total.count() << " ms " << std::endl;
			}
		}

	} else {
		std::cout << "please indicate rom file" << std::endl;
	}

	std::cout << "COUCOU" << std::endl;

	display.free();
	return 0;
}
