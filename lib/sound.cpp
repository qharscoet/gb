#include "sound.h"
#include "options.h"

#include <cstring>

extern emu_options options;

inline bool get_bit(uint8_t val, uint8_t b)
{
	return val & (1 << b);
}

Sound::Sound(std::span<uint8_t, 0x30> regs)
:registers(regs), wave(&regs[0xA], &regs[0x20]), square1(&regs[0x00]), square2(&regs[0x5]), noise(&regs[0xF]), sample_timer(CLOCKSPEED / SAMPLERATE)
{
	buffer.reserve(BUFFER_SIZE);
	buffer.clear();
}


void Sound::step(uint8_t cycles)
{
	// cycles *= 4;
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
					square1.length_tick();
					square2.length_tick();
					wave.length_tick();
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

			/* Update NR52 channel status on reading */
			registers[NR52 - 0xFF10] = (registers[NR52 - 0xFF10] & ~(1 << 0)) | (square1.is_on() << 0);
			registers[NR52 - 0xFF10] = (registers[NR52 - 0xFF10] & ~(1 << 1)) | (square2.is_on() << 1);
			registers[NR52 - 0xFF10] = (registers[NR52 - 0xFF10] & ~(1 << 2)) | (wave.is_on() << 2);
			registers[NR52 - 0xFF10] = (registers[NR52 - 0xFF10] & ~(1 << 3)) | (noise.is_on() << 3);

			// We only get samples every SAMPLERATE cycles
			if (--sample_timer == 0)
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
					bool options_ch_enabled = (&options.sound.channel1)[i];
					float fsample = (options_ch_enabled?ch_samples[i]:0.0f) * 2.0f / 15 - 1.0f;
					if (get_bit(output_sel, i))
						sample.sample_r += fsample;
					if (get_bit(output_sel, i + 4))
						sample.sample_l += fsample;
				}

				sample.sample_l *= ((registers[NR50 - 0xFF10] & 0x70) >> 4) + 1;
				sample.sample_r *= ((registers[NR50 - 0xFF10] & 0x07)) + 1;

				/* 4 channels output each at [-1.0, 1.0] and can be multiplied by up to 8 so total values are [-32.0, 32.0]
				 so we put them back to [-1.0, 1.0] */
				sample.sample_l /= 32.0f;
				sample.sample_r /= 32.0f;

				buffer.push_back(sample);
			}
		}
	}
}

const float* Sound::get_sound_data() const
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
	// return &((char*)registers)[addr];
	return (char*)(registers.data() + addr);
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
	} else if(addr >= 0xFF1F && addr <= 0xFF23)
	{
		noise.write_reg(addr - 0xFF1F, val);
	} else if (addr == NR52) {
		// If we write 0 in bit 7
		if(!get_bit(val, 7))
		{
			std::memset(registers.data(), 0, 0x30);
		}
		//If we write 1 in bit 7 and bit 7 was 0
		else if(!get_bit(registers[NR52 - 0xFF10], 7))
		{
			frame_sequencer = 0;
			frame_seq_step = 0;
		}

		//Only bit 7 is writable in NR52
		val &= 0x80;
	}

	registers[addr - 0xFF10] = val;

}

uint8_t Sound::read_reg(uint16_t addr) const
{
	return registers[addr - 0xFF10];
}