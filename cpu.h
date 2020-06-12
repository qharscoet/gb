#include <cstdint>

#include "memory.h"

typedef uint16_t a16;

class CPU
{
private:
	enum class r8: uint8_t {
		A, F, B, C, D, E, H, L
	};

	enum class r16: uint8_t{
		AF, BC, DE, HL, SP, PC
	};

	enum class flag_id: uint8_t {
		Z, N, H, C
	};

	// uint8_t registers[8]; //registers A F B C D E H L
	// uint16_t sp;
	// uint16_t pc;

	uint16_t registers[6];
	uint16_t* sp;
	uint16_t* pc;

	uint8_t* flags;

	Memory* memory;
	//char *memory;

	uint8_t read_r8(r8 r);
	uint16_t read_r16(r16 r);
	void write_r8(r8 r, uint8_t value);
	void write_r16(r16 r, uint16_t value);

	uint8_t read_pc8();
	uint16_t read_pc16();

	void set_flag(flag_id f);
	void reset_flag(flag_id f);
	bool get_flag(flag_id f);

	void inc(r16 r);
	void dec(r16 r);

	// 8 bits ld
	void ld(r8 r1, uint8_t value);
	void ld(r8 r1, r8 r2);
	void ld(r8 r1, r16 r2);
	void ld(r16 r1, r8 r2);
	void ld(r8 r, a16 addr);  // addr is a pointer
	void ld(a16 addr, r8 r);

	// 16 bits ld
	void ld(r16 r, uint16_t value);
	void ld(r16 r1, r16 r2);
	void ldhl(r16 r, int8_t n);

	void ldd(r8 r1, r16 r2);
	void ldd(r16 r1, r8 r2);
	void ldi(r8 r1, r16 r2);
	void ldi(r16 r1, r8 r2);


public:

	CPU();
	CPU(Memory* memory);
	~CPU();

	void step();
};