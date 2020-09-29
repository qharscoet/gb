#ifndef __SOUND_CHANNEL_H__
#define __SOUND_CHANNEL_H__

#include <cstdint>
#include <vector>

#include <span>

class Sound;

class Channel
{
protected:
	uint16_t timer;
	uint8_t volume;
	uint8_t position;

	uint8_t length_counter;
	bool length_enabled;

	std::span<uint8_t, 5> registers;

public:
	Channel(uint8_t* data);
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
protected:

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
	SquareChannel(uint8_t *data);
	~SquareChannel() = default;

	// void init();
	void step();
	uint8_t get_sample();
	void write_reg(uint16_t addr, uint8_t val);

	void trigger();

	void vol_envelope();
};


class SquareSweepChannel : public SquareChannel {

	private:

	public:
		SquareSweepChannel(uint8_t *data);
		~SquareSweepChannel() = default;
};


class ChannelWave : public Channel
{
private:

	static const uint16_t wave_addr = 0xFF30;
	std::span<uint8_t, 32> wave_data;

public:
	ChannelWave(uint8_t *data, uint8_t *wave_data);
	~ChannelWave() = default;

	// void init();
	void step();
	uint8_t get_sample();
	void write_reg(uint16_t addr, uint8_t val);

	void trigger();
};

#endif