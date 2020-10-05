#include "sound.h"
#include "options.h"
extern emu_options options;

inline bool get_bit(uint8_t val, uint8_t b)
{
	return val & (1 << b);
}

Sound::Sound()
:wave(&registers[0xA], &registers[0x20]), square1(&registers[0x00]), square2(&registers[0x5]), noise(&registers[0xF]), sample_timer(CLOCKSPEED / SAMPLERATE)
{
	buffer.reserve(BUFFER_SIZE);
	buffer.clear();
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
					noise.length_tick();
				}

				if((frame_seq_step & 0x3) == 0x2 )
				{
					square1.sweep();
				}

				if(frame_seq_step == 7)
				{
					square1.vol_envelope();
					square2.vol_envelope();
					noise.vol_envelope();
				}


				frame_seq_step = (frame_seq_step + 1) & 0x7;
			}

			wave.step();
			square1.step();
			square2.step();
			noise.step();


			// We only get samples every SAMPLERATE cycles
			if(--sample_timer == 0)
			{
				sample_timer = CLOCKSPEED/SAMPLERATE;

				uint8_t output_sel = registers[NR51 - 0xFF10];

				sample sample = {0,0};

				const uint8_t ch_samples[4] = {
					square1.get_sample(),
					square2.get_sample(),
					wave.get_sample(),
					noise.get_sample()
				};

				for(uint8_t i = 0; i < 4; i++)
				{
					if((&options.sound.channel1)[i])
					{
						if(get_bit(output_sel, i))
							sample.sample_r += ch_samples[i];
						if(get_bit(output_sel, i + 4))
							sample.sample_l += ch_samples[i];
					}
				}

				buffer.push_back(sample);
			}
		}
	}
}

const uint8_t* Sound::get_sound_data() const
{
	if(buffer.size() >= BUFFER_SIZE)
	{
		return &(buffer[0].sample_l);
	}

	return nullptr;
}

void Sound::clear_data()
{
	buffer.clear();
}

char* const Sound::get_data(uint16_t addr)
{
	return &((char*)registers)[addr];
}

void Sound::write_reg(uint16_t addr, uint8_t val)
{
	registers[addr - 0xFF10] = val;
	if(addr >= 0xFF10 && addr <= 0xFF14)
	{
		square1.write_reg(addr - 0xFF10, val);
	} else if(addr >= 0xFF15 && addr <= 0xFF19)
	{
		square2.write_reg(addr - 0xFF15,val);
	} else if(addr >= 0xFF1A && addr <= 0xFF1E)
	{
		wave.write_reg(addr - 0xFF1A, val);
	} else if(addr >= 0xFF1F && addr <= 0xFF23)
	{
		noise.write_reg(addr - 0xFF1F, val);
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

}

uint8_t Sound::read_reg(uint16_t addr) const
{
	return registers[addr - 0xFF10];
}