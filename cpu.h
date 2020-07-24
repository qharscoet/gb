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

	bool ime;

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

	void set_dec_flags(uint8_t val);
	void dec(r8 r);
	void dec(r16 r);
	void decp(r16 r);

	void set_inc_flags(uint8_t val);
	void inc(r8 r);
	void inc(r16 r);
	void incp(r16 r); //Increment memory at address stored in r

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
	void ld(uint16_t addr, r16 r);

	void ldd(r8 r1, r16 r2);
	void ldd(r16 r1, r8 r2);
	void ldi(r8 r1, r16 r2);
	void ldi(r16 r1, r8 r2);

	void push(r16 r);
	void pop(r16 r);

	// 8 bits ALU
	/* TODO : use only one function and put the right read in the switch call ?
		Less code but maybe more difficult to optimise later
	*/
	void add(r8 r, uint8_t val);
	void add(r8 r1, r8 r2);
	void add(r8 r1, r16 r2);
	void addc(r8 r1, r8 r2);

	void sub(r8 r, uint8_t val);
	void sub(r8 r1, r8 r2);
	void sub(r8 r1, r16 r2);
	void subc(r8 r1, r8 r2);

	// binary AND
	void band(r8 r, uint8_t val);
	void band(r8 r, r8 r2);
	void band(r8 r, r16 r2);

	// binary OR

	void bor(r8 r, uint8_t val);
	void bor(r8 r, r8 r2);
	void bor(r8 r, r16 r2);

	// binary xOR

	void bxor(r8 r, uint8_t val);
	void bxor(r8 r, r8 r2);
	void bxor(r8 r, r16 r2);

	void cp(r8 r, uint8_t val);
	void cp(r8 r, r8 r2);
	void cp(r8 r, r16 r2);

	// 16bit ALU
	void add(r16 r, uint16_t val);
	void add(r16 r1, r16 r2);

	//Misc
	uint8_t swap(uint8_t val);
	void swap(r8 r);
	void swap(r16 r);

	void daa();
	void cpl();
	void ccf();
	void scf();

	void halt();
	void stop();
	void di();
	void ei();



public:

	CPU();
	CPU(Memory* memory);
	~CPU();

	void step();
};