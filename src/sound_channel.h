#include "memory.h"

class Channel
{
protected:
	uint16_t timer;
	const Memory* memory;
public:
	Channel(const Memory* memory);
	~Channel() = default;

	//virtual void init();
	virtual void step() = 0;
	virtual uint8_t get_sample() = 0;

	virtual void trigger() = 0;
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
	uint8_t length_counter;

public:
	ChannelWave(const Memory* memory);
	~ChannelWave() = default;

	void init();
	void step();
	uint8_t get_sample();

	void trigger();
};
