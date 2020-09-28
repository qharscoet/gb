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
	uint8_t volume;
	uint8_t position;

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

class SquareChannel : public Channel
{
private:
	static const uint16_t NRX1 = 0xFF16;
	static const uint16_t NRX2 = 0xFF17;
	static const uint16_t NRX3 = 0xFF18;
	static const uint16_t NRX4 = 0xFF19;

	const bool duty_pattern[4][8] = {
		{true, false, false, false, false, false, false, false},
		{true, true, false, false, false, false, false, false},
		{true, true, true, true, false, false, false, false},
		{true, true, true, true, true, true, false, false},
	};

	uint8_t duty;
	uint8_t envelope_timer;
	bool envelope_enabled;

public:
	SquareChannel(const Sound *apu);
	~SquareChannel() = default;

	// void init();
	void step();
	uint8_t get_sample();
	void write_reg(uint16_t addr, uint8_t val);

	void trigger();

	void vol_envelope();
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

public:
	ChannelWave(const Sound* apu);
	~ChannelWave() = default;

	// void init();
	void step();
	uint8_t get_sample();
	void write_reg(uint16_t addr, uint8_t val);

	void trigger();
};

#endif