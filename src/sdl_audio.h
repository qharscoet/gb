#ifndef __SDL_AUDIO_H__
#define __SDL_AUDIO_H__

#include "audio.h"
#include <SDL2/SDL.h>

class SDL_Audio : public Audio
{
private:
	SDL_AudioDeviceID audio_dev;
protected:
	static const int BUFFER_SIZE = 1024;
public:
	SDL_Audio(/* args */);
	~SDL_Audio();

	virtual int audio_init();
	virtual void play_audio(const float *samples);
};


#endif