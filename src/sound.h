
#ifndef __SOUND_H__
#define __SOUND_H__

#include "memory.h"
#include "sound_channel.h"

class Sound
{
private:
	static constexpr uint32_t SAMPLERATE = 48000;
	static constexpr uint32_t CLOCKSPEED = 4194304;
	static constexpr uint32_t BUFFER_SIZE = 4096;

	const Memory* memory;
	ChannelWave wave;

	uint32_t sample_timer;
	std::vector<uint8_t> buffer;

	// void channel_3(uint16_t values[32]);

public:
	Sound(Memory* memory);
	~Sound() = default;


	void step(uint8_t cycles);

	const uint8_t* get_sound_data() const;
	void clear_data();
};

#endif
