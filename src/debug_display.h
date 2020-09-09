#include "display.h"

#include <SDL2/SDL.h>
#undef main

#include "emulator.h"


class Debug_Display : public Display
{
private:
	SDL_GLContext gl_context;
	SDL_Window *sdlWindow;

	Emulator& emu;

	enum class keys {
		A,B,START,SELECT,
		RIGHT,LEFT,UP,DOWN,
	};

	uint8_t keystate;

	void update_keystate();

public:
	Debug_Display(Emulator& emu);
	~Debug_Display();

	void set_title(std::string str);

	int init();

	void clear();
	void update(const uint32_t *pixels);
	void render();

	bool handle_events();
	uint8_t get_keystate();
};