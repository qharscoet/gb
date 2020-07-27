#ifndef _EXTENDED_SET_H_
#define _EXTENDED_SET_H_

using CPU_func = void (CPU::*)();

#define SWAP(id) (&CPU::swap<CPU::r8::id>)
#define SWAP16(id) (&CPU::swap<CPU::r16::id>)
#define RLC(id) (&CPU::rlc<CPU::r8::id>)
#define RLC16(id) (&CPU::rlc<CPU::r16::id>)
#define RL(id) (&CPU::rl<CPU::r8::id>)
#define RL16(id) (&CPU::rl<CPU::r16::id>)
#define RRC(id) (&CPU::rrc<CPU::r8::id>)
#define RRC16(id) (&CPU::rrc<CPU::r16::id>)
#define RR(id) (&CPU::rr<CPU::r8::id>)
#define RR16(id) (&CPU::rr<CPU::r16::id>)
#define SLA(id) (&CPU::sla<CPU::r8::id>)
#define SLA16(id) (&CPU::sla<CPU::r16::id>)
#define SRA(id) (&CPU::sra<CPU::r8::id>)
#define SRA16(id) (&CPU::sra<CPU::r16::id>)
#define SRL(id) (&CPU::sra<CPU::r8::id>)
#define SRL16(id) (&CPU::sra<CPU::r16::id>)


CPU_func CPU::extended_set[256] = {
	/*		X0		X1		X2		X3		X4		X5		X6		X7			X8		X9		XA		XB		XC		XD		XE		XF*/
/*0X */ 	RLC(B),	RLC(C),	RLC(D),	RLC(E),	RLC(H),	RLC(L),	RLC16(HL),RLC(A),	RRC(B),	RRC(C),	RRC(D),	RRC(E),	RRC(H),	RRC(L),	RRC16(HL),	RRC(A),
/*1X */ 	RL(B),	RL(C),	RL(D),	RL(E),	RL(H),	RL(L),	RL16(HL),RL(A),		RR(B),	RR(C),	RR(D),	RR(E),	RR(H),	RR(L),	RR16(HL),	RR(A),
/*2X */ 	SLA(B),	SLA(C),	SLA(D),	SLA(E),	SLA(H),	SLA(L),	SLA16(HL),SLA(A),	SRA(B),	SRA(C),	SRA(D),	SRA(E),	SRA(H),	SRA(L),	SRA16(HL),	SRA(A),
/*3X */ 	SWAP(B),SWAP(C),SWAP(D),SWAP(E),SWAP(H),SWAP(L),SWAP16(HL),SWAP(A),	SRL(B),	SRL(C),	SRL(D),	SRL(E),	SRL(H),	SRL(L),	SRL16(HL),	SRL(A),

};


#endif