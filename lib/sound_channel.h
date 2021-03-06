#ifndef __SOUND_CHANNEL_H__
#define __SOUND_CHANNEL_H__

#include <cstdint>
#include <vector>

#include <span>

class Channel
{
protected:
	uint32_t timer;
	uint8_t volume;
	uint8_t position;

	//can go up to 256
	uint16_t length_counter;
	bool length_enabled;

	bool enabled;

	std::span<uint8_t, 5> registers;

	inline uint32_t frequency() { return ((registers[4] & 0x07) << 8) | registers[3]; };

	Channel(uint8_t* data);
	~Channel() = default;
public:

	//virtual void init();
	virtual void step() = 0;
	virtual uint8_t get_sample() = 0;

	virtual void trigger() = 0;
	virtual void write_reg(uint16_t addr, uint8_t val) = 0;

	void length_tick();
	virtual bool is_on();
};

class EnvelopeChannel : public Channel
{
	protected:
		uint8_t envelope_timer;
		bool envelope_enabled;

		EnvelopeChannel(uint8_t *data);
		~EnvelopeChannel() = default;

	public:
		virtual void trigger();
		void vol_envelope();

		virtual void write_reg(uint16_t addr, uint8_t val);
};

class SquareChannel : public EnvelopeChannel
{
protected:
	const uint8_t duty_pattern[4]{
		0b1000'0000, 0b1100'0000, 0b1111'0000, 0b1111'1100};

	uint8_t duty;
	// uint8_t envelope_timer;
	// bool envelope_enabled;

public:
	SquareChannel(uint8_t *data);
	~SquareChannel() = default;

	// void init();
	void step();
	virtual uint8_t get_sample();
	virtual void write_reg(uint16_t addr, uint8_t val);

	virtual void trigger();


};


class SquareSweepChannel : public SquareChannel {

	private:
		uint8_t sweep_timer;
		bool sweep_enabled;
		uint16_t shadow_frequency;
		bool sweep_sub;

		void sweep_calculation(bool update);

	public:
		SquareSweepChannel(uint8_t *data);
		~SquareSweepChannel() = default;

		uint8_t get_sample();
		void write_reg(uint16_t addr, uint8_t val);
		void trigger();

		void sweep();
};


class WaveChannel : public Channel
{
private:

	static const uint16_t wave_addr = 0xFF30;
	std::span<uint8_t, 32> wave_data;

	uint8_t sample_buffer;

public:
	WaveChannel(uint8_t *data, uint8_t *wave_data);
	~WaveChannel() = default;

	// void init();
	void step();
	uint8_t get_sample();
	void write_reg(uint16_t addr, uint8_t val);

	void trigger();
	bool is_on();
};


class NoiseChannel: public EnvelopeChannel
{
	private:
		const uint8_t base_divisor[8] = {8, 16, 32, 48, 64, 80, 96, 112};

		uint16_t lfsr;

		void run_lfsr();

		inline bool width_mode() { return registers[3] & 0x8; };
		inline uint32_t frequency() { return base_divisor[registers[3] & 0x7] << (registers[3] >> 4); };

	public:
		NoiseChannel(uint8_t *data);
		~NoiseChannel() = default;

		void step();
		uint8_t get_sample();

		void trigger();
};

#endif