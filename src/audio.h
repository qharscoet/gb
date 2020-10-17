#ifndef __AUDIO_H__
#define __AUDIO_H__

class Audio
{
private:
public:
	virtual ~Audio() {};

	virtual int audio_init() = 0;
	virtual void play_audio(const float* samples) = 0;
};


#endif