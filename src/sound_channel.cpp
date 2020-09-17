#include "sound_channel.h"

inline bool get_bit(uint8_t val, uint8_t b)
{
	return val & (1 << b);
}

Channel::Channel(const Memory* memory)
:timer(0)
{
	this->memory = memory;
}

ChannelWave::ChannelWave(const Memory* memory)
:Channel(memory), position(0)
{
	const uint16_t freq = ((memory->read_8bits(NR34) & 0x03) << 8) | memory->read_8bits(NR33);
	timer = (2048 - freq) * 2;
}

void ChannelWave::step()
{
	const uint8_t enabled = get_bit(memory->read_8bits(NR30), 7);
	if(enabled)
	{
		timer--;
		if(timer == 0)
		{
			const uint16_t freq = ((memory->read_8bits(NR34) & 0x03) << 8) | memory->read_8bits(NR33);
			timer = (2048 - freq) * 2;
			position = (position + 1) & 0x1F; //Reset at 32;
		}
	}
}

uint8_t ChannelWave::get_sample()
{
	const uint8_t val = memory->read_8bits(wave_addr + (position >> 1));

	return position & 1? val >> 4: val & 0x0F;
}