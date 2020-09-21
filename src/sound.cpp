#include "sound.h"

inline bool get_bit(uint8_t val, uint8_t b)
{
	return val & (1 << b);
}

Sound::Sound(Memory* memory)
:wave(memory), sample_timer(CLOCKSPEED / SAMPLERATE)
{
	this->memory = memory;
	buffer.reserve(BUFFER_SIZE);
}

// void Sound::channel_3(uint16_t values[32])
// {
// 	static const uint16_t NR30 = 0xFF1A;
// 	static const uint16_t NR31 = 0xFF1B;
// 	static const uint16_t NR32 = 0xFF1C;
// 	static const uint16_t NR33 = 0xFF1D;
// 	static const uint16_t NR34 = 0xFF1E;

// 	const uint8_t enabled = get_bit(memory->read_8bits(NR30), 7);

// 	static uint16_t timer = 0;


// 	if(enabled)
// 	{
// 		const uint8_t length = memory->read_8bits(NR31);

// 		const uint8_t nr34 = memory->read_8bits(NR34);
// 		const bool length_enabled = get_bit(nr34, 6);
// 		const bool init_flag = get_bit(nr34, 7);

// 		const uint16_t freq = ((nr34 & 0x03)  << 8) | memory->read_8bits(NR33);

// 		for(int i = 0; i < 16; i++)
// 		{
// 			const uint8_t val = memory->read_8bits(0xFF30 + i);
// 			values[i] = val << 4;
// 			values[i + i] = val & 0xF;
// 		}
// 	}
// }

void Sound::step(uint8_t cycles)
{
	cycles *= 4;
	for(uint8_t i = 0; i < cycles; i++)
	{
		wave.step();
		//TODO : add other channels


		// We only get samples every SAMPLERATE cycles
		if(--sample_timer == 0)
		{
			sample_timer = CLOCKSPEED/SAMPLERATE;

			uint8_t sample = wave.get_sample();
			buffer.push_back(sample);
		}
	}
}

const uint8_t* Sound::get_sound_data() const
{
	if(buffer.size() >= BUFFER_SIZE)
	{
		return &buffer[0];
	}

	return nullptr;
}

void Sound::clear_data()
{
	buffer.clear();
}
