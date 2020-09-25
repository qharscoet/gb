
#ifndef __SOUND_H__
#define __SOUND_H__

#include "sound_channel.h"

class Sound
{
private:
	static constexpr uint32_t SAMPLERATE = 48000;
	static constexpr uint32_t CLOCKSPEED = 4194304;
	static constexpr uint32_t BUFFER_SIZE = 4096;


	static const uint16_t NR50 = 0xFF24;
	static const uint16_t NR51 = 0xFF25;
	static const uint16_t NR52 = 0xFF26;

	// clocked at 512 Hz, which means every 8192 clocks for the CU at 4Mhz
	uint16_t frame_sequencer = 0;
	uint8_t frame_seq_step = 0;

	//const Memory* memory;
	ChannelWave wave;

	//mapped to 0xFF10 - 0xFF3F
	char registers[0x3F];

	uint32_t sample_timer;
	std::vector<uint8_t> buffer;

	// void channel_3(uint16_t values[32]);

public:
	Sound();
	~Sound() = default;


	void step(uint8_t cycles);

	const uint8_t* get_sound_data() const;
	void clear_data();

	void write_reg(uint16_t addr, uint8_t value);
	uint8_t read_reg(uint16_t addr) const;
};

#endif
