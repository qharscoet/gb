#include <cstdint>

class CPU
{
private:
	uint8_t registers[8]; //registers A F B C D E H L
	uint16_t sp;
	uint16_t pc;

	uint8_t flags;

public:
	enum class register_name {
		A, F, B, C, D, E, H, L
	};

	enum class flags_name {
		Z, N, H, C
	};

	CPU(/* args */);
	~CPU();
};