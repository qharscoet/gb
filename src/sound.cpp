#include "sound.h"

inline bool get_bit(uint8_t val, uint8_t b)
{
	return val & (1 << b);
}

Sound::Sound()
:wave(&registers[0xA], &registers[0x20]), square1(&registers[0x00]), square2(&registers[0x5]), sample_timer(CLOCKSPEED / SAMPLERATE)
{
	buffer.reserve(BUFFER_SIZE);
}


void Sound::step(uint8_t cycles)
{
	cycles *= 4;
	for(uint8_t i = 0; i < cycles; i++)
	{
		if (get_bit(registers[NR52 - 0xFF10], 7))
		{
			// reset at 8192 cycles, which is 512Hz
			frame_sequencer = (frame_sequencer + 1) & 0x1FFF;
			if(frame_sequencer == 0)
			{
				if(!(frame_seq_step & 1))
				{
					wave.length_tick();
					square1.length_tick();
					square2.length_tick();
				}

				if(frame_seq_step == 7)
				{
					square1.vol_envelope();
					square2.vol_envelope();
				}


				frame_seq_step = (frame_seq_step + 1) & 0x7;
			}

			wave.step();
			square1.step();
			square2.step();
			//TODO : add other channels


			// We only get samples every SAMPLERATE cycles
			if(--sample_timer == 0)
			{
				sample_timer = CLOCKSPEED/SAMPLERATE;

				uint8_t sample = 0;

				uint8_t wave_smpl = wave.get_sample();
				uint8_t sq1_smpl = square1.get_sample();
				uint8_t sq2_smpl = square2.get_sample();

				sample += wave_smpl;
				sample += sq1_smpl;
				sample += sq2_smpl;

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
	if(addr >= 0xFF10 && addr <= 0xFF14)
	{
		square1.write_reg(addr - 0xFF10, val);
	} else if(addr >= 0xFF15 && addr <= 0xFF19)
	{
		square2.write_reg(addr - 0xFF15,val);
	} else if(addr >= 0xFF1A && addr <= 0xFF1E)
	{
		wave.write_reg(addr - 0xFF1A, val);
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