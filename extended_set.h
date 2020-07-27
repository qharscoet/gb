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

#define BIT(id, b) (&CPU::bit<CPU::r8::id, b>)
#define BIT16(id, b) (&CPU::bit<CPU::r16::id, b>)
#define SET(id, b) (&CPU::set<CPU::r8::id, b>)
#define SET16(id, b) (&CPU::set<CPU::r16::id, b>)
#define RES(id, b) (&CPU::res<CPU::r8::id, b>)
#define RES16(id, b) (&CPU::res<CPU::r16::id, b>)


CPU_func CPU::extended_set[256] = {
	/*		X0		X1		X2		X3		X4		X5		X6		X7			X8		X9		XA		XB		XC		XD		XE		XF*/
/*0X */ 	RLC(B),	RLC(C),	RLC(D),	RLC(E),	RLC(H),	RLC(L),	RLC16(HL),RLC(A),	RRC(B),	RRC(C),	RRC(D),	RRC(E),	RRC(H),	RRC(L),	RRC16(HL),	RRC(A),
/*1X */ 	RL(B),	RL(C),	RL(D),	RL(E),	RL(H),	RL(L),	RL16(HL),RL(A),		RR(B),	RR(C),	RR(D),	RR(E),	RR(H),	RR(L),	RR16(HL),	RR(A),
/*2X */ 	SLA(B),	SLA(C),	SLA(D),	SLA(E),	SLA(H),	SLA(L),	SLA16(HL),SLA(A),	SRA(B),	SRA(C),	SRA(D),	SRA(E),	SRA(H),	SRA(L),	SRA16(HL),	SRA(A),
/*3X */ 	SWAP(B),SWAP(C),SWAP(D),SWAP(E),SWAP(H),SWAP(L),SWAP16(HL),SWAP(A),	SRL(B),	SRL(C),	SRL(D),	SRL(E),	SRL(H),	SRL(L),	SRL16(HL),	SRL(A),

/*4X */		BIT(B,0),BIT(C,0),BIT(D,0),BIT(E,0),BIT(H,0),BIT(L,0),BIT16(HL,0),BIT(A,0),	BIT(B,1),BIT(C,1),BIT(D,1),BIT(E,1),BIT(H,1),BIT(L,1),BIT16(HL,1),BIT(A,1),
/*5X */		BIT(B,2),BIT(C,2),BIT(D,2),BIT(E,2),BIT(H,2),BIT(L,2),BIT16(HL,2),BIT(A,2),	BIT(B,3),BIT(C,3),BIT(D,3),BIT(E,3),BIT(H,3),BIT(L,3),BIT16(HL,3),BIT(A,3),
/*6X */		BIT(B,4),BIT(C,4),BIT(D,4),BIT(E,4),BIT(H,4),BIT(L,4),BIT16(HL,4),BIT(A,4),	BIT(B,5),BIT(C,5),BIT(D,5),BIT(E,5),BIT(H,5),BIT(L,5),BIT16(HL,5),BIT(A,5),
/*7X */		BIT(B,6),BIT(C,6),BIT(D,6),BIT(E,6),BIT(H,6),BIT(L,6),BIT16(HL,6),BIT(A,6),	BIT(B,7),BIT(C,7),BIT(D,7),BIT(E,7),BIT(H,7),BIT(L,7),BIT16(HL,7),BIT(A,7),

/*8X */		RES(B,0),RES(C,0),RES(D,0),RES(E,0),RES(H,0),RES(L,0),RES16(HL,0),RES(A,0),	RES(B,1),RES(C,1),RES(D,1),RES(E,1),RES(H,1),RES(L,1),RES16(HL,1),RES(A,1),
/*9X */		RES(B,2),RES(C,2),RES(D,2),RES(E,2),RES(H,2),RES(L,2),RES16(HL,2),RES(A,2),	RES(B,3),RES(C,3),RES(D,3),RES(E,3),RES(H,3),RES(L,3),RES16(HL,3),RES(A,3),
/*AX */		RES(B,4),RES(C,4),RES(D,4),RES(E,4),RES(H,4),RES(L,4),RES16(HL,4),RES(A,4),	RES(B,5),RES(C,5),RES(D,5),RES(E,5),RES(H,5),RES(L,5),RES16(HL,5),RES(A,5),
/*BX */		RES(B,6),RES(C,6),RES(D,6),RES(E,6),RES(H,6),RES(L,6),RES16(HL,6),RES(A,6),	RES(B,7),RES(C,7),RES(D,7),RES(E,7),RES(H,7),RES(L,7),RES16(HL,7),RES(A,7),

/*CX */		SET(B,0),SET(C,0),SET(D,0),SET(E,0),SET(H,0),SET(L,0),SET16(HL,0),SET(A,0),	SET(B,1),SET(C,1),SET(D,1),SET(E,1),SET(H,1),SET(L,1),SET16(HL,1),SET(A,1),
/*DX */		SET(B,2),SET(C,2),SET(D,2),SET(E,2),SET(H,2),SET(L,2),SET16(HL,2),SET(A,2),	SET(B,3),SET(C,3),SET(D,3),SET(E,3),SET(H,3),SET(L,3),SET16(HL,3),SET(A,3),
/*EX */		SET(B,4),SET(C,4),SET(D,4),SET(E,4),SET(H,4),SET(L,4),SET16(HL,4),SET(A,4),	SET(B,5),SET(C,5),SET(D,5),SET(E,5),SET(H,5),SET(L,5),SET16(HL,5),SET(A,5),
/*FX */		SET(B,6),SET(C,6),SET(D,6),SET(E,6),SET(H,6),SET(L,6),SET16(HL,6),SET(A,6),	SET(B,7),SET(C,7),SET(D,7),SET(E,7),SET(H,7),SET(L,7),SET16(HL,7),SET(A,7),
};


#endif