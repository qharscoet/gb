#include "sound_channel.h"

inline bool get_bit(uint8_t val, uint8_t b)
{
	return val & (1 << b);
}

Channel::Channel(uint8_t* data)
:timer(0), enabled(true), registers(data, 5), length_enabled(false)
{}

void Channel::length_tick()
{
	if (length_enabled && length_counter > 0)
	{
		length_counter--;
	}
}

EnvelopeChannel::EnvelopeChannel(uint8_t *data)
: Channel(data)
{}

void EnvelopeChannel::vol_envelope()
{
	if (envelope_enabled && --envelope_timer == 0)
	{
		uint8_t envelope_period = registers[2] & 0x7;
		envelope_timer = envelope_period ? envelope_period : 8;

		if (envelope_period != 0)
		{
			int8_t new_volume = volume + (get_bit(registers[2], 3) ? 1 : -1);
			if (new_volume >= 0 && new_volume <= 15)
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

void EnvelopeChannel::write_reg(uint16_t addr, uint8_t val)
{
	switch (addr)
	{
	case 1:
		length_counter = (val & 0x3F);
		break;
	case 2:
		// volume = (val & 0xF0) >> 4;
		break;
	case 3:
		break;
	case 4:
		if (get_bit(val, 7))
			trigger();

		length_enabled = get_bit(val, 6);
		break;
	}
}

void EnvelopeChannel::trigger()
{
	envelope_timer = registers[2] & 0x7;
	if (envelope_timer == 0)
		envelope_timer = 8;

	envelope_enabled = true;
	volume = (registers[2] & 0xF0) >> 4;
}

SquareChannel::SquareChannel(uint8_t *data)
: EnvelopeChannel(data)
{
	timer = (2048 - frequency()) * 4;
}

void SquareChannel::step()
{
	// if ((length_enabled && length_counter != 0) || !length_enabled)
	// {
		if (--timer == 0)
		{
			timer = (2048 - frequency()) * 4;
			position = (position + 1) & 0x07; //Reset at 8;
		}
	// }
}
uint8_t SquareChannel::get_sample()
{
	if ((length_enabled && length_counter != 0) || !length_enabled)
	{
		return get_bit(duty_pattern[duty], position) * volume;
	}
	return 0;
}

void SquareChannel::write_reg(uint16_t addr, uint8_t val)
{
	EnvelopeChannel::write_reg(addr,val);

	if(addr == 1)
	{
		duty = (val & 0xC0) >> 6;
	}
}

void SquareChannel::trigger()
{
	EnvelopeChannel::trigger();

	if (length_counter == 0)
	{
		length_counter = 64;
	}

	timer = (timer & 0x3) | (((2048 - frequency()) * 4) & ~0x3);
	position = 0;
}


SquareSweepChannel::SquareSweepChannel(uint8_t *data)
: SquareChannel(data), sweep_enabled(false),shadow_frequency(0)
{}

void SquareSweepChannel::write_reg(uint16_t addr, uint8_t val)
{
	if(addr == 0 && sweep_enabled && sweep_sub && !get_bit(val,3))
		enabled = false;

	SquareChannel::write_reg(addr,val);
}

void SquareSweepChannel::sweep_calculation(bool update)
{
	const uint8_t sweep_shift = registers[0] & 0x7;
	sweep_sub = get_bit(registers[0], 3);
	uint16_t delta = shadow_frequency >> sweep_shift;
	uint16_t new_freq = shadow_frequency + (sweep_sub ? -delta : delta);

	if (new_freq > 2047)
	{
		enabled = false;
	}
	else if (sweep_shift != 0 && update)
	{
		shadow_frequency = new_freq;
		registers[3] = new_freq & 0xFF;
		registers[4] = (registers[4] & 0xF8) | (new_freq >> 8); //Change only last 3bits
	}
}


void SquareSweepChannel::sweep()
{
	if (--sweep_timer == 0)
	{
		uint8_t sweep_period = (registers[0] & 0x70) >> 4;
		sweep_timer = sweep_period ? sweep_period : 8;

		if (sweep_enabled && sweep_period != 0)
		{
			sweep_calculation(true);
			sweep_calculation(false);
		}

	}
}

uint8_t SquareSweepChannel::get_sample()
{
	if(enabled)
		return SquareChannel::get_sample();

	return 0;
}

void SquareSweepChannel::trigger()
{
	SquareChannel::trigger();

	shadow_frequency = frequency();
	const uint8_t sweep_period = (registers[0] & 0x70) >> 4;
	sweep_timer = sweep_period ? sweep_period : 8;


	const uint8_t sweep_shift = registers[0] & 0x7;

	sweep_enabled = (sweep_period != 0 && sweep_shift != 0);
	sweep_sub = false;
	enabled = true;

	if(sweep_shift != 0)
	{
		sweep_calculation(false);
	}
}






/* Wave Channel */

WaveChannel::WaveChannel(uint8_t *data, uint8_t *wave)
:Channel(data), wave_data(wave, 32)
{
	timer = (2048 - frequency()) * 2;
	position = 0;
	sample_buffer = 0;
}

void WaveChannel::write_reg(uint16_t addr, uint8_t val)
{
	switch(addr)
	{
		case 1:
			length_counter = val;
			break;
		case 2:
			volume = (val & 0x60) >> 5; // We get bits 5 and 6;
			break;
		case 3:
			break;
		case 4:
			if (get_bit(val, 7))
				trigger();

			length_enabled = get_bit(val, 6);
			break;
	}
}

void WaveChannel::step()
{

	const uint8_t enabled = get_bit(registers[0], 7);

	if(enabled && ((length_enabled && length_counter != 0) || !length_enabled))
	{
		timer--;
		if(timer == 0)
		{
			timer = (2048 - frequency()) * 2;
			position = (position + 1) & 0x1F; //Reset at 32;

			const uint8_t val = wave_data[position >> 1];
			sample_buffer = position & 1 ? val & 0x0F : val >> 4;
		}
	}
}

uint8_t WaveChannel::get_sample()
{
	const uint8_t enabled = get_bit(registers[0], 7);

	if(enabled && ((length_enabled && length_counter != 0) || !length_enabled))
	{
		return sample_buffer >> (volume != 0 ? volume - 1 : 4);
	}

	return 0;
}


void WaveChannel::trigger()
{
	if (length_counter == 0)
	{
		length_counter = 256;
	}

	timer = (2048 - frequency()) * 2;
	position = 0;
}



/* Noise Channel */

NoiseChannel::NoiseChannel(uint8_t *data)
:EnvelopeChannel(data), lfsr(0x7FFF)
{
	timer = frequency();
}

void NoiseChannel::run_lfsr()
{
	bool result = get_bit(lfsr,0) ^ get_bit(lfsr, 1);
	lfsr >>= 1;
	lfsr |= result << 14;

	if(width_mode())
	{
		// If in 7bits mode, we also put the result into the 7th bit
		lfsr = (lfsr & ~0x40) | ( result << 6);
	}
}

void NoiseChannel::step()
{
	if ((length_enabled && length_counter != 0) || !length_enabled)
	{
		if (--timer == 0)
		{
			timer = frequency();
			run_lfsr();
		}
	}
}


uint8_t NoiseChannel::get_sample()
{
	if ((length_enabled && length_counter != 0) || !length_enabled)
	{
		return !(lfsr & 1) * volume;
	}
	return 0;
}

void NoiseChannel::trigger()
{
	EnvelopeChannel::trigger();

	lfsr = 0x7FFF;
	timer = frequency();
}