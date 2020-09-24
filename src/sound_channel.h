#ifndef __SOUND_CHANNEL_H__
#define __SOUND_CHANNEL_H__

#include <cstdint>
#include <vector>

class Sound;

class Channel
{
protected:
	const Sound* apu;

	uint16_t timer;

	uint8_t length_counter;
	bool length_enabled;

public:
	Channel(const Sound* apu);
	~Channel() = default;

	//virtual void init();
	virtual void step() = 0;
	virtual uint8_t get_sample() = 0;

	virtual void trigger() = 0;
	virtual void write_reg(uint16_t addr, uint8_t val) = 0;

	void length_tick();
};


class ChannelWave : public Channel
{
private:
	static const uint16_t NR30 = 0xFF1A;
	static const uint16_t NR31 = 0xFF1B;
	static const uint16_t NR32 = 0xFF1C;
	static const uint16_t NR33 = 0xFF1D;
	static const uint16_t NR34 = 0xFF1E;

	static const uint16_t wave_addr = 0xFF30;

	uint8_t position;
	uint8_t volume;

public:
	ChannelWave(const Sound* apu);
	~ChannelWave() = default;

	void init();
	void step();
	uint8_t get_sample();
	void write_reg(uint16_t addr, uint8_t val);

	void trigger();
};

#endif