
#ifndef __SOUND_H__
#define __SOUND_H__

#include "sound_channel.h"

class Sound
{
private:
	static constexpr uint32_t SAMPLERATE = 48000;
	static constexpr uint32_t CLOCKSPEED = 4194304;
	static constexpr uint32_t BUFFER_SIZE = 1024;


	static const uint16_t NR50 = 0xFF24;
	static const uint16_t NR51 = 0xFF25;
	static const uint16_t NR52 = 0xFF26;

	struct sample {
		float sample_l;
		float sample_r;
	};

	// clocked at 512 Hz, which means every 8192 clocks for the CU at 4Mhz
	uint16_t frame_sequencer = 0;
	uint8_t frame_seq_step = 0;

	//const Memory* memory;
	SquareSweepChannel square1;
	SquareChannel square2;
	WaveChannel wave;
	NoiseChannel noise;

	//mapped to 0xFF10 - 0xFF3F
	//uint8_t registers[0x2F];
	std::span<uint8_t, 0x2F> registers;

	uint32_t sample_timer;
	std::vector<sample> buffer;

	// void channel_3(uint16_t values[32]);

public:
	Sound(std::span<uint8_t, 0x2F> regs);
	~Sound() = default;


	void step(uint8_t cycles);

	const float* get_sound_data() const;
	void clear_data();

	char* const get_data(uint16_t addr);

	void write_reg(uint16_t addr, uint8_t value);
	uint8_t read_reg(uint16_t addr) const;
};

#endif
