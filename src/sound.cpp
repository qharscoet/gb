#include "sound.h"

inline bool get_bit(uint8_t val, uint8_t b)
{
	return val & (1 << b);
}

Sound::Sound()
:wave(this), square2(this), sample_timer(CLOCKSPEED / SAMPLERATE)
{
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
		if (get_bit(registers[NR52 - 0xFF10], 7))
		{
			// reset at 8192
			frame_sequencer = (frame_sequencer + 1) & 0x2000;
			if(frame_sequencer == 0)
			{
				if(!(frame_seq_step & 1))
				{
					wave.length_tick();
					square2.length_tick();
				}


				frame_seq_step = (frame_seq_step + 1) & 0x7;
			}

			wave.step();
			square2.step();
			//TODO : add other channels


			// We only get samples every SAMPLERATE cycles
			if(--sample_timer == 0)
			{
				sample_timer = CLOCKSPEED/SAMPLERATE;

				uint8_t sample = wave.get_sample();
				sample += square2.get_sample();
				buffer.push_back(sample);
			}
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


void Sound::write_reg(uint16_t addr, uint8_t val)
{
	if(addr >= 0xFF15 && addr <= 0xFF19)
	{
		square2.write_reg(addr,val);
	} else if(addr >= 0xFF1A && addr <= 0xFF1E)
	{
		wave.write_reg(addr, val);
	} else if (addr == NR52) {
		if(!get_bit(val, 7))
		{
			memset(registers, 0, 0x3F);
		} else if(!get_bit(registers[NR52 - 0xFF10], 7))
		{
			frame_sequencer = 0;
			frame_seq_step = 0;
		}
	}

	registers[addr - 0xFF10] = val;
}

uint8_t Sound::read_reg(uint16_t addr) const
{
	return registers[addr - 0xFF10];
}