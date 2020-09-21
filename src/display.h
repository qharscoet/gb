#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include <iostream>

#include "emulator.h"

class Display
{
protected:
	static const uint8_t LCD_WIDTH = 160;
	static const uint8_t LCD_HEIGHT = 144;

public:
	virtual ~Display() {};

	virtual void set_title(std::string str) = 0;

	virtual int init() = 0;

	virtual void clear() = 0;
	virtual void update(const uint32_t* pixels) = 0;
	virtual void render() = 0;

	virtual bool handle_events(Emulator &emu) = 0;
	virtual uint8_t get_keystate() = 0;

	virtual void play_audio(const uint8_t* samples) = 0;
};

#endif