#include "sound_channel.h"

inline bool get_bit(uint8_t val, uint8_t b)
{
	return val & (1 << b);
}

Channel::Channel(uint8_t* data)
:timer(0), registers(data, 5)
{}

void Channel::length_tick()
{
	if (length_enabled && length_counter > 0)
	{
		length_counter--;
	}
}

SquareChannel::SquareChannel(uint8_t *data)
:Channel(data)
{
	const uint16_t freq = ((registers[4] & 0x03) << 8) | registers[3];
	timer = (2048 - freq) * 4;
}

void SquareChannel::step()
{
	if ((length_enabled && length_counter != 0) || !length_enabled)
	{
		timer--;
		if (timer == 0)
		{
			const uint16_t freq = ((registers[4] & 0x07) << 8) | registers[3];
			timer = (2048 - freq) * 4;
			position = (position + 1) & 0x07; //Reset at 8;
		}
	}
}
uint8_t SquareChannel::get_sample()
{
	if ((length_enabled && length_counter != 0) || !length_enabled)
	{
		return duty_pattern[duty][position] * volume;
	}
	return 0;
}
void SquareChannel::write_reg(uint16_t addr, uint8_t val)
{
	if(addr == NRX1)
	{
		length_counter = (val & 0x3F);
		duty = (val & 0xC0) >> 6;
	} else if (addr == NRX2) {
		// volume = (val & 0xF0) >> 4;
	} else if(addr == NRX4){
		if (get_bit(val, 7))
			trigger();

		length_enabled = get_bit(val, 6);
	}
}

void SquareChannel::trigger()
{
	if (length_counter == 0)
	{
		length_counter = 64;
	}

	const uint16_t freq = ((registers[4] & 0x03) << 8) | registers[3];
	timer = (2048 - freq) * 4;
	position = 0;

	envelope_timer = registers[2] & 0x7;
	if(envelope_timer == 0) envelope_timer = 8;

	envelope_enabled = true;
	volume = (registers[2] & 0xF0) >> 4;
}

void SquareChannel::vol_envelope()
{
	if (envelope_enabled && --envelope_timer == 0)
	{
		uint8_t envelope_period = registers[2] & 0x7;
		envelope_timer = envelope_period?envelope_period:8;

		if(envelope_period != 0)
		{
			int8_t new_volume = volume + (get_bit(registers[2], 3)? 1:-1);
			if(new_volume >= 0 && new_volume <= 15)
			{
				volume = new_volume;
			}
			else
			{
				envelope_enabled = false;
			}
		}

	}
}

ChannelWave::ChannelWave(uint8_t *data, uint8_t *wave)
:Channel(data), wave_data(wave, 32)
{
	const uint16_t freq = ((registers[4] & 0x03) << 8) | registers[3];
	timer = (2048 - freq) * 2;

	position = 0;
}

void ChannelWave::write_reg(uint16_t addr, uint8_t val)
{
	if(addr == NR31)
	{
		length_counter = val;
	} else if (addr == NR32) {
		volume = (val & 0x60) >> 5; // We get bits 5 and 6;
	} else if(addr == NR34){
		if (get_bit(val, 7))
			trigger();

		length_enabled = get_bit(val, 6);
	}
}

void ChannelWave::step()
{

	const uint8_t enabled = get_bit(registers[0], 7);

	if(enabled && ((length_enabled && length_counter != 0) || !length_enabled))
	{
		timer--;
		if(timer == 0)
		{
			const uint16_t freq = ((registers[4] & 0x07) << 8) | registers[3];
			timer = (2048 - freq) * 2;
			position = (position + 1) & 0x1F; //Reset at 32;
		}
	}
}

uint8_t ChannelWave::get_sample()
{
	const uint8_t enabled = get_bit(registers[0], 7);

	if(enabled && ((length_enabled && length_counter != 0) || !length_enabled))
	{
		const uint8_t val = wave_data[position >> 1];
		uint8_t sample = position & 1 ? val & 0x0F : val >> 4;
		sample >>= (volume != 0 ? volume - 1 : 4) ;

		return sample;
	}

	return 0;
}


void ChannelWave::trigger()
{
	if (length_counter == 0)
	{
		length_counter = 256;
	}

	const uint16_t freq = ((registers[4] & 0x03) << 8) | registers[3];
	timer = (2048 - freq) * 2;
	position = 0;
}