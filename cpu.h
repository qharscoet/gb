#include <cstdint>

class CPU
{
private:
	enum class r8_name: char {
		A, F, B, C, D, E, H, L
	};

	enum class r16_name: char{
		AF, BC, DE, HL
	};

	enum class flags_name {
		Z, N, H, C
	};

	uint8_t registers[8]; //registers A F B C D E H L
	uint16_t sp;
	uint16_t pc;

	uint8_t flags;

	char* memory;

	void write_r8_register(r8_name r, uint8_t value);
	uint8_t read_r8_register(r8_name r);
	uint16_t read_r16_register(r16_name r);
	uint8_t read_8_bit_from_memory(uint16_t addr);
	uint16_t read_16_bit_from_memory(uint16_t addr);
	void write_8_bits_to_memory(uint16_t addr, uint8_t value);

	void ld(r8_name r1, uint8_t value);
	void ld(r8_name r1, r8_name r2);
	void ld(r8_name r1, r16_name r2);
	void ld(r16_name r1, r8_name r2);

public:

	CPU();
	CPU(char* memory);
	~CPU();

	void step();
};