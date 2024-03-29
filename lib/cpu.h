#ifndef _CPU_H_
#define _CPU_H_

#include <cstdint>
#include <variant>

#include "memory.h"

typedef uint16_t a16;

inline uint8_t set_bit(uint8_t val, uint8_t b)
{
	return val | (1 << b);
}

inline uint8_t res_bit(uint8_t val, uint8_t b)
{
	return val & ~(1 << b);
}

class CPU
{
private:

	static const int CLOCKSPEED = 4194304;

	enum class r8: uint8_t {
		A, F, B, C, D, E, H, L
	};

	enum class r16: uint8_t{
		AF, BC, DE, HL, SP, PC
	};


	using Param8bits = std::variant<r8, r16>;

	enum class flag_id: uint8_t {
		Z = 7 , N = 6, H = 5, C = 4,
	};

	// uint8_t registers[8]; //registers A F B C D E H L
	// uint16_t pc;

	uint16_t registers[6];
	uint16_t* pc;

	uint8_t* flags;

	//Set to true if a jump/call/ret conditionnal instr has executed or not
	bool test_true;

	bool ime = false;
	bool ime_scheduled = false;

	bool halted = false;

	//Timers
	uint8_t divider_cycle_count = 0;
	uint16_t timer_cycle_count = 0;

	Memory* memory;
	//char *memory;

	uint8_t execute();
	void step_divider(uint8_t cycles);
	void step_timers(uint8_t cycles);
	void step_interrupts();

	uint8_t read_r8(r8 r);
	uint16_t read_r16(r16 r);
	void write_r8(r8 r, uint8_t value);
	void write_r16(r16 r, uint16_t value);

	uint8_t read_pc8();
	uint16_t read_pc16();

	uint8_t read_8bits(Param8bits p);
	void write_8bits(Param8bits p, uint8_t val);

	void set_flag(flag_id f);
	inline void set_flag(flag_id f, bool b);
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
	void ld(r16 r, uint8_t val);

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
	void add(r8 r, uint8_t val, bool carry);
	void add(r8 r1, r8 r2);
	void add(r8 r1, r16 r2);
	void addc(r8 r, uint8_t val);
	void addc(r8 r1, r16 r2);
	void addc(r8 r1, r8 r2);

	void sub(r8 r, uint8_t val, bool cflag);
	void sub(r8 r1, r8 r2);
	void sub(r8 r1, r16 r2);
	void subc(r8 r, uint8_t val);
	void subc(r8 r1, r16 r2);
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
	void add_sp(r16 r, int8_t val);
	void add(r16 r, uint16_t val);
	void add(r16 r1, r16 r2);

	//Misc
	uint8_t swap(uint8_t val);

	template<CPU::r8 r> void swap();
	template<CPU::r16 r>void swap16();
	//template<CPU::Param8bits p> void swap();

	void daa();
	void cpl();
	void ccf();
	void scf();

	void halt();
	void stop();
	void di();
	void ei();


	//Rotate and shifts
	uint8_t rotate(uint8_t val, bool left, bool carry);
	void rotate_a(bool left, bool carry);
	void rotate_r(r8 r, bool left, bool carry);
	void rotate_p(uint16_t addr, bool left, bool carry);
	void rlca();
	void rla();
	void rrca();
	void rra();
	template<CPU::r8 r> void rlc();
	template<CPU::r8 r> void rl();
	template<CPU::r8 r> void rrc();
	template<CPU::r8 r> void rr();
	template<CPU::r16 r> void rlc16();
	template<CPU::r16 r> void rl16();
	template<CPU::r16 r> void rrc16();
	template<CPU::r16 r> void rr16();

	uint8_t shift(uint8_t val, bool left, bool arithmetic);
	template<CPU::r8 r> void sla();
	template<CPU::r8 r> void sra();
	template<CPU::r8 r> void srl();
	template<CPU::r16 r> void sla16();
	template<CPU::r16 r> void sra16();
	template<CPU::r16 r> void srl16();

	//Bit Ops
	void bit(uint8_t val, uint8_t b);
	template<CPU::r8 r, uint8_t b> void bit();
	template<CPU::r16 r, uint8_t b> void bit16();

	template<CPU::r8 r, uint8_t b> void set();
	template<CPU::r16 r, uint8_t b> void set16();

	template<CPU::r8 r, uint8_t b> void res();
	template<CPU::r16 r, uint8_t b> void res16();

	// Jumps

	void jp();
	void jnz();
	void jz();
	void jnc();
	void jc();

	void jhl();

	void jr();
	void jrnz();
	void jrz();
	void jrnc();
	void jrc();

	//Calls
	void call_addr(uint16_t addr);
	void call();
	void callnz();
	void callz();
	void callnc();
	void callc();

	//Restart
	void rst(uint8_t n);

	//Returns
	void ret();
	void retnz();
	void retz();
	void retnc();
	void retc();
	void reti();

	static uint8_t instructions_cycles[256];

	using CPU_func = void (CPU::*)();
	static CPU_func extended_set[256];
	static uint8_t extended_cycles[256];

public:

	CPU();
	CPU(Memory* memory);
	~CPU() = default;

	void reset();
	void init();
	uint8_t step();

	bool is_double_speed() const;
	uint16_t get_pc() const;
	
};

inline void CPU::set_flag(flag_id f, bool b)
{
	if (b)
		set_flag(f);
	else
		reset_flag(f);
}

template <CPU::r8 r>
void CPU::swap()
{
	write_r8(r, swap(read_r8(r)));
}

template <CPU::r16 r>
void CPU::swap16()
{
	uint16_t addr = read_r16(r);
	uint8_t val = memory->read_8bits(addr);
	memory->write_8bits(addr, swap(val));
}

template <CPU::r8 r>
void CPU::rlc()
{
	rotate_r(r, true, true);
}

template <CPU::r8 r>
void CPU::rl()
{
	rotate_r(r, true, false);
}

template <CPU::r8 r>
void CPU::rrc()
{
	rotate_r(r, false, true);
}

template <CPU::r8 r>
void CPU::rr()
{
	rotate_r(r, false, false);
}

template <CPU::r16 r>
void CPU::rlc16()
{
	rotate_p(read_r16(r), true, true);
}

template <CPU::r16 r>
void CPU::rl16()
{
	rotate_p(read_r16(r), true, false);
}

template <CPU::r16 r>
void CPU::rrc16()
{
	rotate_p(read_r16(r), false, true);
}

template <CPU::r16 r>
void CPU::rr16()
{
	rotate_p(read_r16(r), false, false);
}

template <CPU::r8 r>
void CPU::sla()
{
	write_r8(r, shift(read_r8(r), true, true));
}

template <CPU::r8 r>
void CPU::sra()
{
	write_r8(r, shift(read_r8(r), false, true));
}

template <CPU::r8 r>
void CPU::srl()
{
	write_r8(r, shift(read_r8(r), false, false));
}

template <CPU::r16 r>
void CPU::sla16()
{
	uint16_t addr = read_r16(r);
	uint8_t val = memory->read_8bits(addr);
	val = shift(val, true, true);
	memory->write_8bits(addr, val);
}

template <CPU::r16 r>
void CPU::sra16()
{
	uint16_t addr = read_r16(r);
	uint8_t val = memory->read_8bits(addr);
	val = shift(val, false, true);
	memory->write_8bits(addr, val);
}

template <CPU::r16 r>
void CPU::srl16()
{
	uint16_t addr = read_r16(r);
	uint8_t val = memory->read_8bits(addr);
	val = shift(val, false, false);
	memory->write_8bits(addr, val);
}

template <CPU::r8 r, uint8_t b>
void CPU::bit()
{
	bit(read_r8(r), b);
}

template <CPU::r16 r, uint8_t b>
void CPU::bit16()
{
	bit(memory->read_8bits(read_r16(r)), b);
}


template <CPU::r8 r, uint8_t b>
void CPU::set()
{
	write_r8(r, set_bit(read_r8(r), b));
}

template <CPU::r16 r, uint8_t b>
void CPU::set16()
{
	uint16_t addr = read_r16(r);
	uint8_t val = memory->read_8bits(addr);
	memory->write_8bits(addr, set_bit(val, b));
}


template <CPU::r8 r, uint8_t b>
void CPU::res()
{
	write_r8(r, res_bit(read_r8(r), b));
}

template <CPU::r16 r, uint8_t b>
void CPU::res16()
{
	uint16_t addr = read_r16(r);
	uint8_t val = memory->read_8bits(addr);
	memory->write_8bits(addr, res_bit(val, b));
}

#endif