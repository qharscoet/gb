#ifndef _EXTENDED_SET_H_
#define _EXTENDED_SET_H_

using CPU_func = void (CPU::*)();

#define SWAP(id) (&CPU::swap<CPU::r8::id>)
#define SWAP16(id) (&CPU::swap<CPU::r16::id>)

CPU_func CPU::extended_set[256] = {
	/*		X0		X1		X2		X3		X4		X5		X6		X7		X8		X9		XA		XB		XC		XD		XE		XF*/
/*0X */ 	nullptr, nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
/*1X */ 	nullptr, nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
/*2X */ 	nullptr, nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
/*3X */ 	SWAP(B), SWAP(C),SWAP(D),SWAP(E),SWAP(H),SWAP(L),SWAP16(HL),SWAP(A),nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,

};


#endif