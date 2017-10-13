#include <stdio.h>

#include "z80.h"
#include "Disassemble.h"
#include "lib.h"

#define MXT_BUFSIZE 80

#define MXT_PV_NONE 0
#define MXT_PV_PARITY 1
#define MXT_PV_OVERFLOW 2

#define MXT_ARITH_NONE 0
#define MXT_ARITH_ADD 1
#define MXT_ARITH_SUB 2

namespace z80 {

template<typename T>
void swap(T& t1, T& t2)
{
	T tt(t1);
	t1 = t2;
	t2 = tt;
}

void Z80::printStatus(std::ostream& os)
{
	char buf[MXT_BUFSIZE];
	auto printfn = [&os, &buf](const char *name, uint16_t val)
		{ snprintf(buf, MXT_BUFSIZE, "%-4s: 0x%04X (%u)\n", name, val, val); os << buf; };

	printfn("PC", PC);
	printfn("SP", SP);
	printfn("AF", AF);
	printfn("AF'", AFp);
	printfn("BC", BC);
	printfn("BC'", BCp);
	printfn("DE", DE);
	printfn("DE'", DEp);
	printfn("HL", HL);
	printfn("HL'", HLp);
	printfn("IR", IR);
	printfn("IX", IX);
	printfn("IY", IY);

	os << "\n"
	   << "	  |  S  |  Z  |	 |  H  | P/V |	 |  N  |  C \n"
	   << "------+-----+-----+-----+-----+-----+-----+-----+-----\n"
	   << "FLAGS ";
	
	for(int i = 7 ; i >= 0 ; --i)
	{
		os << "|  " << ((F() & (1 << i)) ? "1" : "0") << "  ";
	}

	os << "\n\n"
	   << "Interrupts " << (int_ ? "enabled" : "disabled") << "\n"
	   << (halted_ ? "Halted" : "Running") << "\n";
}

void Z80::printRAM(std::ostream& os, addr_t start, size_t len)
{
	char buf[MXT_BUFSIZE];
	addr_t i1 = start / 16, i2 = (start + len + 15) / 16;

	os << "	 | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n"
	   << "-----+-------------------------------------------------\n";
	
	for(addr_t i = i1 ; i < i2 ; ++i)
	{
		snprintf(buf, MXT_BUFSIZE, "%04X", i * 16); os << buf << " | ";

		for(size_t j = 0 ; j < 16 ; ++j)
		{
			if(j + i * 16 < start) os << "   ";
			else if(j + i * 16 >= start + len) break;
			else { snprintf(buf, MXT_BUFSIZE, "%02X ", ram_[j + i * 16]); os << buf; }
		}

		os << "\n";
	}
}

void Z80::reset(void)
{
	int_ = false;
	halted_ = false;
	PC = 0;
	IR = 0;
}

void Z80::loadRAM(addr_t start, const Program& prg)
{
	const uint8_t *data = prg.data();
	size_t len = prg.length();

	for(size_t i = 0 ; i < len ; ++i)
	{
		ram_[(start + i) % 0x10000] = data[i];
	}
}

void Z80::registerPeripheral(port_t p, Peripheral& o)
{
	periphs_[p >> 4] = &o;
}

void Z80::execute(void)
{
	bool do_int = int_ && interrupted_;

	interrupted_ = false;

	byte_t ins = loadB(PC);
	uint8_t t8;
	uint16_t t16;

	if(do_int)
	{
		ins = 0xFF; // rst38
		halted_ = false;
	}
	else if(halted_)
	{
		return;
	}
	else
	{
		++PC;
	}

	switch(ins)
	{
		case 0x00: // nop
			break;
		case 0x01: // ld bc,d16
			BC = loadW();
			break;
		case 0x02: // ld (bc),a
			storeB(BC, A());
			break;
		case 0x03: // inc bc
			++BC;
			break;
		case 0x04: // inc b
			set_inc_flags(++B());
			break;
		case 0x05: // dec b
			set_dec_flags(--B());
			break;
		case 0x06: // ld b,d8
			B() = loadB();
			break;
		case 0x07: // rlca
			rlca();
			break;
		case 0x08: // ex af,af'
			swap(AF, AFp);
			break;
		case 0x09: // add hl,bc
			addHL(BC);
			break;
		case 0x0A: // ld a,(bc)
			A() = loadB(BC);
			break;
		case 0x0B: // dec bc
			--BC;
			break;
		case 0x0C: // inc c
			set_inc_flags(++C());
			break;
		case 0x0D: // dec c
			set_dec_flags(--C());
			break;
		case 0x0E: // ld c,d8
			C() = loadB();
			break;
		case 0x0F: // rrca
			rrca();
			break;
		case 0x10: // djnz d8
			if(--B()) jr(); else ++PC;
			break;
		case 0x11: // ld de,d16
			DE = loadW();
			break;
		case 0x12: // ld (de),d8
			storeB(DE, A());
			break;
		case 0x13: // inc DE
			++DE;
			break;
		case 0x14: // inc d
			set_inc_flags(++D());
			break;
		case 0x15: // dec d
			set_dec_flags(--D());
			break;
		case 0x16: // ld d,d8
			D() = loadB();
			break;
		case 0x17: // rla
			rla();
			break;
		case 0x18: // jr s8
			jr();
			break;
		case 0x19: // add hl,de
			addHL(DE);
			break;
		case 0x1A: // ld a,(de)
			A() = loadB(DE);
			break;
		case 0x1B: // dec de
			--DE;
			break;
		case 0x1C: // inc e
			set_inc_flags(++E());
			break;
		case 0x1D: // dec e
			set_dec_flags(--E());
		case 0x1E: // ld e,d8
			E() = loadB();
			break;
		case 0x1F: // rra
			rra();
			break;
		case 0x20: // jr nz,s8
			if(!(F() & FLAG_Z)) jr(); else ++PC;
			break;
		case 0x21: // ld hl,d16
			HL = loadW();
			break;
		case 0x22: // ld (a16),hl
			storeW(loadW(), HL);
			break;
		case 0x23: // inc hl
			++HL;
			break;
		case 0x24: // inc h
			set_inc_flags(++H());
			break;
		case 0x25: // dec h
			set_dec_flags(--H());
			break;
		case 0x26: // ld h,d8
			H() = loadB();
			break;
		case 0x27: // daa
			daa();
			break;
		case 0x28: // jr z,s8
			if(F() & FLAG_Z) jr(); else ++PC;
			break;
		case 0x29: // add hl,hl
			addHL(HL);
			break;
		case 0x2A: // ld hl,(a16)
			HL = loadW(loadW());
			break;
		case 0x2B: // dec hl
			--HL;
			break;
		case 0x2C: // inc l
			set_inc_flags(++L());
			break;
		case 0x2D: // dec l
			set_dec_flags(--L());
			break;
		case 0x2E: // ld l,d8
			L() = loadB();
			break;
		case 0x2F: // cpl
			A() = ~A();
			F() |= FLAG_N | FLAG_H;
			break;
		case 0x30: // jr nc,s8
			if(!(F() & FLAG_C)) jr(); else ++PC;
			break;
		case 0x31: // ld sp,d16
			SP = loadW();
			break;
		case 0x32: // ld (a16),a
			storeB(loadW(), A());
			break;
		case 0x33: // inc sp
			++SP;
			break;
		case 0x34: // inc (hl)
			storeB(HL, loadB(HL) + 1);
			set_inc_flags(loadB(HL));
			break;
		case 0x35: // dec (hl)
			storeB(HL, loadB(HL) - 1);
			set_dec_flags(loadB(HL));
			break;
		case 0x36: // ld (hl),d8
			storeB(HL, loadB());
			break;
		case 0x37: // scf
			F() |= FLAG_C;
			F() &= ~FLAG_N & ~FLAG_H;
			break;
		case 0x38: // jr c,s8
			if(F() & FLAG_C) jr(); else ++PC;
			break;
		case 0x39: // add hl,sp
			addHL(SP);
			break;
		case 0x3A: // ld a,(a16)
			A() = loadB(loadW());
			break;
		case 0x3B: // dec sp
			--SP;
			break;
		case 0x3C: // inc a
			set_inc_flags(++A());
			break;
		case 0x3D: // dec a
			set_dec_flags(--A());
			break;
		case 0x3E: // ld a,d8
			A() = loadB();
			break;
		case 0x3F: // ccf
			F() = (F() & FLAG_C) ? ((F() & ~FLAG_C) | FLAG_H) : ((F() | FLAG_C) & ~FLAG_H);
			F() &= ~FLAG_N;
			break;
		case 0x40: // ld b,b
			B() = B();
			break;
		case 0x41: // ld b,c
			B() = C();
			break;
		case 0x42: // ld b,d
			B() = D();
			break;
		case 0x43: // ld b,e
			B() = E();
			break;
		case 0x44: // ld b,h
			B() = H();
			break;
		case 0x45: // ld b,l
			B() = L();
			break;
		case 0x46: // ld b,(hl)
			B() = loadB(HL);
			break;
		case 0x47: // ld b,a
			B() = A();
			break;
		case 0x48: // ld c,b
			C() = B();
			break;
		case 0x49: // ld c,c
			C() = C();
			break;
		case 0x4A: // ld c,d
			C() = D();
			break;
		case 0x4B: // ld c,e
			C() = E();
			break;
		case 0x4C: // ld c,h
			C() = H();
			break;
		case 0x4D: // ld c,l
			C() = L();
			break;
		case 0x4E: // ld c,(hl)
			C() = loadB(HL);
			break;
		case 0x4F: // ld c,a
			C() = A();
			break;
		case 0x50: // ld d,b
			D() = B();
			break;
		case 0x51: // ld d,c
			D() = C();
			break;
		case 0x52: // ld d,d
			D() = D();
			break;
		case 0x53: // ld d,e
			D() = E();
			break;
		case 0x54: // ld d,h
			D() = H();
			break;
		case 0x55: // ld d,l
			D() = L();
			break;
		case 0x56: // ld d,(hl)
			D() = loadB(HL);
			break;
		case 0x57: // ld d,a
			D() = A();
			break;
		case 0x58: // ld e,b
			E() = B();
			break;
		case 0x59: // ld e,c
			E() = C();
			break;
		case 0x5A: // ld e,d
			E() = D();
			break;
		case 0x5B: // ld e,e
			E() = E();
			break;
		case 0x5C: // ld e,h
			E() = H();
			break;
		case 0x5D: // ld e,l
			E() = L();
			break;
		case 0x5E: // ld e,(hl)
			E() = loadB(HL);
			break;
		case 0x5F: // ld e,a
			E() = A();
			break;
		case 0x60: // ld h,b
			H() = B();
			break;
		case 0x61: // ld h,c
			H() = C();
			break;
		case 0x62: // ld h,d
			H() = D();
			break;
		case 0x63: // ld h,e
			H() = E();
			break;
		case 0x64: // ld h,h
			H() = H();
			break;
		case 0x65: // ld h,l
			H() = L();
			break;
		case 0x66: // ld h,(hl)
			H() = loadB(HL);
			break;
		case 0x67: // ld h,a
			H() = A();
			break;
		case 0x68: // ld l,b
			L() = B();
			break;
		case 0x69: // ld l,c
			L() = C();
			break;
		case 0x6A: // ld l,d
			L() = D();
			break;
		case 0x6B: // ld l,e
			L() = E();
			break;
		case 0x6C: // ld l,h
			L() = H();
			break;
		case 0x6D: // ld l,l
			L() = L();
			break;
		case 0x6E: // ld l,(hl)
			L() = loadB(HL);
			break;
		case 0x6F: // ld l,a
			L() = A();
			break;
		case 0x70: // ld (hl),b
			storeB(HL, B());
			break;
		case 0x71: // ld (hl),c
			storeB(HL, C());
			break;
		case 0x72: // ld (hl),d
			storeB(HL, D());
			break;
		case 0x73: // ld (hl),e
			storeB(HL, E());
			break;
		case 0x74: // ld (hl),h
			storeB(HL, H());
			break;
		case 0x75: // ld (hl),l
			storeB(HL, L());
			break;
		case 0x76: // halt
			halted_ = true;
			break;
		case 0x77: // ld (hl),a
			storeB(HL, A());
			break;
		case 0x78: // ld a,b
			A() = B();
			break;
		case 0x79: // ld a,c
			A() = C();
			break;
		case 0x7A: // ld a,d
			A() = D();
			break;
		case 0x7B: // ld a,e
			A() = E();
			break;
		case 0x7C: // ld a,h
			A() = H();
			break;
		case 0x7D: // ld a,l
			A() = L();
			break;
		case 0x7E: // ld a,(hl)
			A() = loadB(HL);
			break;
		case 0x7F: // ld a,a
			A() = A();
			break;
		case 0x80: // add a,b
			addA(B());
			break;
		case 0x81: // add a,c
			addA(C());
			break;
		case 0x82: // add a,d
			addA(D());
			break;
		case 0x83: // add a,e
			addA(E());
			break;
		case 0x84: // add a,h
			addA(H());
			break;
		case 0x85: // add a,l
			addA(L());
			break;
		case 0x86: // add a,(hl)
			addA(loadB(HL));
			break;
		case 0x87: // add a,a
			addA(A());
			break;
		case 0x88: // adc a,b
			addA(B(), true);
			break;
		case 0x89: // adc a,c
			addA(C(), true);
			break;
		case 0x8A: // adc a,d
			addA(D(), true);
			break;
		case 0x8B: // adc a,e
			addA(E(), true);
			break;
		case 0x8C: // adc a,h
			addA(H(), true);
			break;
		case 0x8D: // adc a,l
			addA(L(), true);
			break;
		case 0x8E: // adc a,(hl)
			addA(loadB(HL), true);
			break;
		case 0x8F: // adc a,a
			addA(A(), true);
			break;
		case 0x90: // sub b
			subA(B());
			break;
		case 0x91: // sub c
			subA(C());
			break;
		case 0x92: // sub d
			subA(D());
			break;
		case 0x93: // sub e
			subA(E());
			break;
		case 0x94: // sub h
			subA(H());
			break;
		case 0x95: // sub l
			subA(L());
			break;
		case 0x96: // sub (hl)
			subA(loadB(HL));
			break;
		case 0x97: // sub a
			subA(A());
			break;
		case 0x98: // sbc a,b
			subA(B(), true);
			break;
		case 0x99: // sbc a,c
			subA(C(), true);
			break;
		case 0x9A: // sbc a,d
			subA(D(), true);
			break;
		case 0x9B: // sbc a,e
			subA(E(), true);
			break;
		case 0x9C: // sbc a,h
			subA(H(), true);
			break;
		case 0x9D: // sbc a,l
			subA(L(), true);
			break;
		case 0x9E: // sbc a,(hl)
			subA(loadB(HL), true);
			break;
		case 0x9F: // sbc a,a
			subA(A(), true);
			break;
		case 0xA0: // and b
			logicA(A() & B(), true);
			break;
		case 0xA1: // and c
			logicA(A() & C(), true);
			break;
		case 0xA2: // and d
			logicA(A() & D(), true);
			break;
		case 0xA3: // and e
			logicA(A() & E(), true);
			break;
		case 0xA4: // and h
			logicA(A() & H(), true);
			break;
		case 0xA5: // and l
			logicA(A() & L(), true);
			break;
		case 0xA6: // and (hl)
			logicA(A() & loadB(HL), true);
			break;
		case 0xA7: // and a
			logicA(A() & A(), true);
			break;
		case 0xA8: // xor b
			logicA(A() ^ B(), false);
			break;
		case 0xA9: // xor c
			logicA(A() ^ C(), false);
			break;
		case 0xAA: // xor d
			logicA(A() ^ D(), false);
			break;
		case 0xAB: // xor e
			logicA(A() ^ E(), false);
			break;
		case 0xAC: // xor h
			logicA(A() ^ H(), false);
			break;
		case 0xAD: // xor l
			logicA(A() ^ L(), false);
			break;
		case 0xAE: // xor (hl)
			logicA(A() ^ loadB(HL), false);
			break;
		case 0xAF: // xor a
			logicA(A() ^ A(), false);
			break;
		case 0xB0: // or b
			logicA(A() | B(), false);
			break;
		case 0xB1: // or c
			logicA(A() | C(), false);
			break;
		case 0xB2: // or d
			logicA(A() | D(), false);
			break;
		case 0xB3: // or e
			logicA(A() | E(), false);
			break;
		case 0xB4: // or h
			logicA(A() | H(), false);
			break;
		case 0xB5: // or l
			logicA(A() | L(), false);
			break;
		case 0xB6: // or (hl)
			logicA(A() | loadB(HL), false);
			break;
		case 0xB7: // or a
			logicA(A() | A(), false);
			break;
		case 0xB8: // cp b
			t8 = A();
			subA(B());
			A() = t8;
			break;
		case 0xB9: // cp c
			t8 = A();
			subA(C());
			A() = t8;
			break;
		case 0xBA: // cp d
			t8 = A();
			subA(D());
			A() = t8;
			break;
		case 0xBB: // cp e
			t8 = A();
			subA(E());
			A() = t8;
			break;
		case 0xBC: // cp h
			t8 = A();
			subA(H());
			A() = t8;
			break;
		case 0xBD: // cp l
			t8 = A();
			subA(L());
			A() = t8;
			break;
		case 0xBE: // cp (hl)
			t8 = A();
			subA(loadB(HL));
			A() = t8;
			break;
		case 0xBF: // cp a
			t8 = A();
			subA(A());
			A() = t8;
			break;
		case 0xC0: // ret nz
			if(!(F() & FLAG_Z)) ret();
			break;
		case 0xC1: // pop bc
			BC = popW();
			break;
		case 0xC2: // jp nz,a16
			t16 = loadW();
			if(!(F() & FLAG_Z)) PC = t16;
			break;
		case 0xC3: // jp a16
			PC = loadW();
			break;
		case 0xC4: // call nz,a16
			t16 = loadW();
			if(!(F() & FLAG_Z)) call(t16);
			break;
		case 0xC5: // push bc
			pushW(BC);
			break;
		case 0xC6: // add a,d8
			addA(loadB());
			break;
		case 0xC7: // rst 00h
			call(0x0000);
			break;
		case 0xC8: // ret z
			if(F() & FLAG_Z) ret();
			break;
		case 0xC9: // ret
			ret();
			break;
		case 0xCA: // jp z,a16
			t16 = loadW();
			if(F() & FLAG_Z) PC = t16;
			break;
		case 0xCB: // BITS
			switch(ins = loadB())
			{
				case 0x00: // rlc b
					rotate_left(B(), BIT_BIT7);
					break;
				case 0x01: // rlc c
					rotate_left(C(), BIT_BIT7);
					break;
				case 0x02: // rlc d
					rotate_left(D(), BIT_BIT7);
					break;
				case 0x03: // rlc e
					rotate_left(E(), BIT_BIT7);
					break;
				case 0x04: // rlc h
					rotate_left(H(), BIT_BIT7);
					break;
				case 0x05: // rlc l
					rotate_left(L(), BIT_BIT7);
					break;
				case 0x06: // rlc (hl)
					t8 = loadB(HL);
					rotate_left(t8, BIT_BIT7);
					storeB(HL, t8);
					break;
				case 0x07: // rlc a
					rotate_left(B(), BIT_BIT7);
					break;
				case 0x08: // rrc b
					rotate_right(B(), BIT_BIT7);
					break;
				case 0x09: // rrc c
					rotate_right(C(), BIT_BIT7);
					break;
				case 0x0A: // rrc d
					rotate_right(D(), BIT_BIT7);
					break;
				case 0x0B: // rrc e
					rotate_right(E(), BIT_BIT7);
					break;
				case 0x0C: // rrc h
					rotate_right(H(), BIT_BIT7);
					break;
				case 0x0D: // rrc l
					rotate_right(L(), BIT_BIT7);
					break;
				case 0x0E: // rrc (hl)
					t8 = loadB(HL);
					rotate_right(t8, BIT_BIT7);
					storeB(HL, t8);
					break;
				case 0x0F: // rrc a
					rotate_right(B(), BIT_BIT7);
					break;
				case 0x10: // rl b
					rotate_left(B(), BIT_CARRY);
					break;
				case 0x11: // rl c
					rotate_left(C(), BIT_CARRY);
					break;
				case 0x12: // rl d
					rotate_left(D(), BIT_CARRY);
					break;
				case 0x13: // rl e
					rotate_left(E(), BIT_CARRY);
					break;
				case 0x14: // rl h
					rotate_left(H(), BIT_CARRY);
					break;
				case 0x15: // rl l
					rotate_left(L(), BIT_CARRY);
					break;
				case 0x16: // rl (hl)
					t8 = loadB(HL);
					rotate_left(t8, BIT_CARRY);
					storeB(HL, t8);
					break;
				case 0x17: // rl a
					rotate_left(B(), BIT_CARRY);
					break;
				case 0x18: // rr b
					rotate_right(B(), BIT_CARRY);
					break;
				case 0x19: // rr c
					rotate_right(C(), BIT_CARRY);
					break;
				case 0x1A: // rr d
					rotate_right(D(), BIT_CARRY);
					break;
				case 0x1B: // rr e
					rotate_right(E(), BIT_CARRY);
					break;
				case 0x1C: // rr h
					rotate_right(H(), BIT_CARRY);
					break;
				case 0x1D: // rr l
					rotate_right(L(), BIT_CARRY);
					break;
				case 0x1E: // rr (hl)
					t8 = loadB(HL);
					rotate_right(t8, BIT_CARRY);
					storeB(HL, t8);
					break;
				case 0x1F: // rr a
					rotate_right(B(), BIT_CARRY);
					break;
				case 0x20: // sla b
					rotate_left(B(), BIT_A);
					break;
				case 0x21: // sla c
					rotate_left(C(), BIT_A);
					break;
				case 0x22: // sla d
					rotate_left(D(), BIT_A);
					break;
				case 0x23: // sla e
					rotate_left(E(), BIT_A);
					break;
				case 0x24: // sla h
					rotate_left(H(), BIT_A);
					break;
				case 0x25: // sla l
					rotate_left(L(), BIT_A);
					break;
				case 0x26: // sla (hl)
					t8 = loadB(HL);
					rotate_left(t8, BIT_A);
					storeB(HL, t8);
					break;
				case 0x27: // sla a
					rotate_left(B(), BIT_A);
					break;
				case 0x28: // sra b
					rotate_right(B(), BIT_A);
					break;
				case 0x29: // sra c
					rotate_right(C(), BIT_A);
					break;
				case 0x2A: // sra d
					rotate_right(D(), BIT_A);
					break;
				case 0x2B: // sra e
					rotate_right(E(), BIT_A);
					break;
				case 0x2C: // sra h
					rotate_right(H(), BIT_A);
					break;
				case 0x2D: // sra l
					rotate_right(L(), BIT_A);
					break;
				case 0x2E: // sra (hl)
					t8 = loadB(HL);
					rotate_right(t8, BIT_A);
					storeB(HL, t8);
					break;
				case 0x2F: // sra a
					rotate_right(B(), BIT_A);
					break;
				case 0x30: // sll b
					rotate_left(B(), BIT_L);
					break;
				case 0x31: // sll c
					rotate_left(C(), BIT_L);
					break;
				case 0x32: // sll d
					rotate_left(D(), BIT_L);
					break;
				case 0x33: // sll e
					rotate_left(E(), BIT_L);
					break;
				case 0x34: // sll h
					rotate_left(H(), BIT_L);
					break;
				case 0x35: // sll l
					rotate_left(L(), BIT_L);
					break;
				case 0x36: // sll (hl)
					t8 = loadB(HL);
					rotate_left(t8, BIT_L);
					storeB(HL, t8);
					break;
				case 0x37: // sll a
					rotate_left(B(), BIT_L);
					break;
				case 0x38: // srl b
					rotate_right(B(), BIT_L);
					break;
				case 0x39: // srl c
					rotate_right(C(), BIT_L);
					break;
				case 0x3A: // srl d
					rotate_right(D(), BIT_L);
					break;
				case 0x3B: // srl e
					rotate_right(E(), BIT_L);
					break;
				case 0x3C: // srl h
					rotate_right(H(), BIT_L);
					break;
				case 0x3D: // srl l
					rotate_right(L(), BIT_L);
					break;
				case 0x3E: // srl (hl)
					t8 = loadB(HL);
					rotate_right(t8, BIT_L);
					storeB(HL, t8);
					break;
				case 0x3F: // srl a
					rotate_right(B(), BIT_L);
					break;
				case 0x40: // bit 0,b
					test_bit(B(), 0);
					break;
				case 0x41: // bit 0,c
					test_bit(C(), 0);
					break;
				case 0x42: // bit 0,d
					test_bit(D(), 0);
					break;
				case 0x43: // bit 0,e
					test_bit(E(), 0);
					break;
				case 0x44: // bit 0,h
					test_bit(H(), 0);
					break;
				case 0x45: // bit 0,l
					test_bit(L(), 0);
					break;
				case 0x46: // bit 0,(hl)
					test_bit(loadB(HL), 0);
					break;
				case 0x47: // bit 0,a
					test_bit(A(), 0);
					break;
				case 0x48: // bit 1,b
					test_bit(B(), 1);
					break;
				case 0x49: // bit 1,c
					test_bit(C(), 1);
					break;
				case 0x4A: // bit 1,d
					test_bit(D(), 1);
					break;
				case 0x4B: // bit 1,e
					test_bit(E(), 1);
					break;
				case 0x4C: // bit 1,h
					test_bit(H(), 1);
					break;
				case 0x4D: // bit 1,l
					test_bit(L(), 1);
					break;
				case 0x4E: // bit 1,(hl)
					test_bit(loadB(HL), 1);
					break;
				case 0x4F: // bit 1,a
					test_bit(A(), 1);
					break;
				case 0x50: // bit 2,b
					test_bit(B(), 2);
					break;
				case 0x51: // bit 2,c
					test_bit(C(), 2);
					break;
				case 0x52: // bit 2,d
					test_bit(D(), 2);
					break;
				case 0x53: // bit 2,e
					test_bit(E(), 2);
					break;
				case 0x54: // bit 2,h
					test_bit(H(), 2);
					break;
				case 0x55: // bit 2,l
					test_bit(L(), 2);
					break;
				case 0x56: // bit 2,(hl)
					test_bit(loadB(HL), 2);
					break;
				case 0x57: // bit 2,a
					test_bit(A(), 2);
					break;
				case 0x58: // bit 3,b
					test_bit(B(), 3);
					break;
				case 0x59: // bit 3,c
					test_bit(C(), 3);
					break;
				case 0x5A: // bit 3,d
					test_bit(D(), 3);
					break;
				case 0x5B: // bit 3,e
					test_bit(E(), 3);
					break;
				case 0x5C: // bit 3,h
					test_bit(H(), 3);
					break;
				case 0x5D: // bit 3,l
					test_bit(L(), 3);
					break;
				case 0x5E: // bit 3,(hl)
					test_bit(loadB(HL), 3);
					break;
				case 0x5F: // bit 3,a
					test_bit(A(), 3);
					break;
				case 0x60: // bit 4,b
					test_bit(B(), 4);
					break;
				case 0x61: // bit 4,c
					test_bit(C(), 4);
					break;
				case 0x62: // bit 4,d
					test_bit(D(), 4);
					break;
				case 0x63: // bit 4,e
					test_bit(E(), 4);
					break;
				case 0x64: // bit 4,h
					test_bit(H(), 4);
					break;
				case 0x65: // bit 4,l
					test_bit(L(), 4);
					break;
				case 0x66: // bit 4,(hl)
					test_bit(loadB(HL), 4);
					break;
				case 0x67: // bit 4,a
					test_bit(A(), 4);
					break;
				case 0x68: // bit 5,b
					test_bit(B(), 5);
					break;
				case 0x69: // bit 5,c
					test_bit(C(), 5);
					break;
				case 0x6A: // bit 5,d
					test_bit(D(), 5);
					break;
				case 0x6B: // bit 5,e
					test_bit(E(), 5);
					break;
				case 0x6C: // bit 5,h
					test_bit(H(), 5);
					break;
				case 0x6D: // bit 5,l
					test_bit(L(), 5);
					break;
				case 0x6E: // bit 5,(hl)
					test_bit(loadB(HL), 5);
					break;
				case 0x6F: // bit 5,a
					test_bit(A(), 5);
					break;
				case 0x70: // bit 6,b
					test_bit(B(), 6);
					break;
				case 0x71: // bit 6,c
					test_bit(C(), 6);
					break;
				case 0x72: // bit 6,d
					test_bit(D(), 6);
					break;
				case 0x73: // bit 6,e
					test_bit(E(), 6);
					break;
				case 0x74: // bit 6,h
					test_bit(H(), 6);
					break;
				case 0x75: // bit 6,l
					test_bit(L(), 6);
					break;
				case 0x76: // bit 6,(hl)
					test_bit(loadB(HL), 6);
					break;
				case 0x77: // bit 6,a
					test_bit(A(), 6);
					break;
				case 0x78: // bit 7,b
					test_bit(B(), 7);
					break;
				case 0x79: // bit 7,c
					test_bit(C(), 7);
					break;
				case 0x7A: // bit 7,d
					test_bit(D(), 7);
					break;
				case 0x7B: // bit 7,e
					test_bit(E(), 7);
					break;
				case 0x7C: // bit 7,h
					test_bit(H(), 7);
					break;
				case 0x7D: // bit 7,l
					test_bit(L(), 7);
					break;
				case 0x7E: // bit 7,(hl)
					test_bit(loadB(HL), 7);
					break;
				case 0x7F: // bit 7,a
					test_bit(A(), 7);
					break;
				case 0x80: // res 0,b
					B() &= ~(1 << 0);
					break;
				case 0x81: // res 0,c
					C() &= ~(1 << 0);
					break;
				case 0x82: // res 0,d
					D() &= ~(1 << 0);
					break;
				case 0x83: // res 0,e
					E() &= ~(1 << 0);
					break;
				case 0x84: // res 0,h
					H() &= ~(1 << 0);
					break;
				case 0x85: // res 0,l
					L() &= ~(1 << 0);
					break;
				case 0x86: // res 0,(hl)
					t8 = loadB(HL);
					t8 &= ~(1 << 0);
					storeB(HL, t8);
					break;
				case 0x87: // res 0,a
					A() &= ~(1 << 0);
					break;
				case 0x88: // res 1,b
					B() &= ~(1 << 1);
					break;
				case 0x89: // res 1,c
					C() &= ~(1 << 1);
					break;
				case 0x8A: // res 1,d
					D() &= ~(1 << 1);
					break;
				case 0x8B: // res 1,e
					E() &= ~(1 << 1);
					break;
				case 0x8C: // res 1,h
					H() &= ~(1 << 1);
					break;
				case 0x8D: // res 1,l
					L() &= ~(1 << 1);
					break;
				case 0x8E: // res 1,(hl)
					t8 = loadB(HL);
					t8 &= ~(1 << 1);
					storeB(HL, t8);
					break;
				case 0x8F: // res 1,a
					A() &= ~(1 << 1);
					break;
				case 0x90: // res 2,b
					B() &= ~(1 << 2);
					break;
				case 0x91: // res 2,c
					C() &= ~(1 << 2);
					break;
				case 0x92: // res 2,d
					D() &= ~(1 << 2);
					break;
				case 0x93: // res 2,e
					E() &= ~(1 << 2);
					break;
				case 0x94: // res 2,h
					H() &= ~(1 << 2);
					break;
				case 0x95: // res 2,l
					L() &= ~(1 << 2);
					break;
				case 0x96: // res 2,(hl)
					t8 = loadB(HL);
					t8 &= ~(1 << 2);
					storeB(HL, t8);
					break;
				case 0x97: // res 2,a
					A() &= ~(1 << 2);
					break;
				case 0x98: // res 3,b
					B() &= ~(1 << 3);
					break;
				case 0x99: // res 3,c
					C() &= ~(1 << 3);
					break;
				case 0x9A: // res 3,d
					D() &= ~(1 << 3);
					break;
				case 0x9B: // res 3,e
					E() &= ~(1 << 3);
					break;
				case 0x9C: // res 3,h
					H() &= ~(1 << 3);
					break;
				case 0x9D: // res 3,l
					L() &= ~(1 << 3);
					break;
				case 0x9E: // res 3,(hl)
					t8 = loadB(HL);
					t8 &= ~(1 << 3);
					storeB(HL, t8);
					break;
				case 0x9F: // res 3,a
					A() &= ~(1 << 3);
					break;
				case 0xA0: // res 4,b
					B() &= ~(1 << 4);
					break;
				case 0xA1: // res 4,c
					C() &= ~(1 << 4);
					break;
				case 0xA2: // res 4,d
					D() &= ~(1 << 4);
					break;
				case 0xA3: // res 4,e
					E() &= ~(1 << 4);
					break;
				case 0xA4: // res 4,h
					H() &= ~(1 << 4);
					break;
				case 0xA5: // res 4,l
					L() &= ~(1 << 4);
					break;
				case 0xA6: // res 4,(hl)
					t8 = loadB(HL);
					t8 &= ~(1 << 4);
					storeB(HL, t8);
					break;
				case 0xA7: // res 4,a
					A() &= ~(1 << 4);
					break;
				case 0xA8: // res 5,b
					B() &= ~(1 << 5);
					break;
				case 0xA9: // res 5,c
					C() &= ~(1 << 5);
					break;
				case 0xAA: // res 5,d
					D() &= ~(1 << 5);
					break;
				case 0xAB: // res 5,e
					E() &= ~(1 << 5);
					break;
				case 0xAC: // res 5,h
					H() &= ~(1 << 5);
					break;
				case 0xAD: // res 5,l
					L() &= ~(1 << 5);
					break;
				case 0xAE: // res 5,(hl)
					t8 = loadB(HL);
					t8 &= ~(1 << 5);
					storeB(HL, t8);
					break;
				case 0xAF: // res 5,a
					A() &= ~(1 << 5);
					break;
				case 0xB0: // res 6,b
					B() &= ~(1 << 6);
					break;
				case 0xB1: // res 6,c
					C() &= ~(1 << 6);
					break;
				case 0xB2: // res 6,d
					D() &= ~(1 << 6);
					break;
				case 0xB3: // res 6,e
					E() &= ~(1 << 6);
					break;
				case 0xB4: // res 6,h
					H() &= ~(1 << 6);
					break;
				case 0xB5: // res 6,l
					L() &= ~(1 << 6);
					break;
				case 0xB6: // res 6,(hl)
					t8 = loadB(HL);
					t8 &= ~(1 << 6);
					storeB(HL, t8);
					break;
				case 0xB7: // res 6,a
					A() &= ~(1 << 6);
					break;
				case 0xB8: // res 7,b
					B() &= ~(1 << 7);
					break;
				case 0xB9: // res 7,c
					C() &= ~(1 << 7);
					break;
				case 0xBA: // res 7,d
					D() &= ~(1 << 7);
					break;
				case 0xBB: // res 7,e
					E() &= ~(1 << 7);
					break;
				case 0xBC: // res 7,h
					H() &= ~(1 << 7);
					break;
				case 0xBD: // res 7,l
					L() &= ~(1 << 7);
					break;
				case 0xBE: // res 7,(hl)
					t8 = loadB(HL);
					t8 &= ~(1 << 7);
					storeB(HL, t8);
					break;
				case 0xBF: // res 7,a
					A() &= ~(1 << 7);
					break;
				case 0xC0: // set 0,b
					B() |=  (1 << 0);
					break;
				case 0xC1: // set 0,c
					C() |=  (1 << 0);
					break;
				case 0xC2: // set 0,d
					D() |=  (1 << 0);
					break;
				case 0xC3: // set 0,e
					E() |=  (1 << 0);
					break;
				case 0xC4: // set 0,h
					H() |=  (1 << 0);
					break;
				case 0xC5: // set 0,l
					L() |=  (1 << 0);
					break;
				case 0xC6: // set 0,(hl)
					t8 = loadB(HL);
					t8 |=  (1 << 0);
					storeB(HL, t8);
					break;
				case 0xC7: // set 0,a
					A() |=  (1 << 0);
					break;
				case 0xC8: // set 1,b
					B() |=  (1 << 1);
					break;
				case 0xC9: // set 1,c
					C() |=  (1 << 1);
					break;
				case 0xCA: // set 1,d
					D() |=  (1 << 1);
					break;
				case 0xCB: // set 1,e
					E() |=  (1 << 1);
					break;
				case 0xCC: // set 1,h
					H() |=  (1 << 1);
					break;
				case 0xCD: // set 1,l
					L() |=  (1 << 1);
					break;
				case 0xCE: // set 1,(hl)
					t8 = loadB(HL);
					t8 |=  (1 << 1);
					storeB(HL, t8);
					break;
				case 0xCF: // set 1,a
					A() |=  (1 << 1);
					break;
				case 0xD0: // set 2,b
					B() |=  (1 << 2);
					break;
				case 0xD1: // set 2,c
					C() |=  (1 << 2);
					break;
				case 0xD2: // set 2,d
					D() |=  (1 << 2);
					break;
				case 0xD3: // set 2,e
					E() |=  (1 << 2);
					break;
				case 0xD4: // set 2,h
					H() |=  (1 << 2);
					break;
				case 0xD5: // set 2,l
					L() |=  (1 << 2);
					break;
				case 0xD6: // set 2,(hl)
					t8 = loadB(HL);
					t8 |=  (1 << 2);
					storeB(HL, t8);
					break;
				case 0xD7: // set 2,a
					A() |=  (1 << 2);
					break;
				case 0xD8: // set 3,b
					B() |=  (1 << 3);
					break;
				case 0xD9: // set 3,c
					C() |=  (1 << 3);
					break;
				case 0xDA: // set 3,d
					D() |=  (1 << 3);
					break;
				case 0xDB: // set 3,e
					E() |=  (1 << 3);
					break;
				case 0xDC: // set 3,h
					H() |=  (1 << 3);
					break;
				case 0xDD: // set 3,l
					L() |=  (1 << 3);
					break;
				case 0xDE: // set 3,(hl)
					t8 = loadB(HL);
					t8 |=  (1 << 3);
					storeB(HL, t8);
					break;
				case 0xDF: // set 3,a
					A() |=  (1 << 3);
					break;
				case 0xE0: // set 4,b
					B() |=  (1 << 4);
					break;
				case 0xE1: // set 4,c
					C() |=  (1 << 4);
					break;
				case 0xE2: // set 4,d
					D() |=  (1 << 4);
					break;
				case 0xE3: // set 4,e
					E() |=  (1 << 4);
					break;
				case 0xE4: // set 4,h
					H() |=  (1 << 4);
					break;
				case 0xE5: // set 4,l
					L() |=  (1 << 4);
					break;
				case 0xE6: // set 4,(hl)
					t8 = loadB(HL);
					t8 |=  (1 << 4);
					storeB(HL, t8);
					break;
				case 0xE7: // set 4,a
					A() |=  (1 << 4);
					break;
				case 0xE8: // set 5,b
					B() |=  (1 << 5);
					break;
				case 0xE9: // set 5,c
					C() |=  (1 << 5);
					break;
				case 0xEA: // set 5,d
					D() |=  (1 << 5);
					break;
				case 0xEB: // set 5,e
					E() |=  (1 << 5);
					break;
				case 0xEC: // set 5,h
					H() |=  (1 << 5);
					break;
				case 0xED: // set 5,l
					L() |=  (1 << 5);
					break;
				case 0xEE: // set 5,(hl)
					t8 = loadB(HL);
					t8 |=  (1 << 5);
					storeB(HL, t8);
					break;
				case 0xEF: // set 5,a
					A() |=  (1 << 5);
					break;
				case 0xF0: // set 6,b
					B() |=  (1 << 6);
					break;
				case 0xF1: // set 6,c
					C() |=  (1 << 6);
					break;
				case 0xF2: // set 6,d
					D() |=  (1 << 6);
					break;
				case 0xF3: // set 6,e
					E() |=  (1 << 6);
					break;
				case 0xF4: // set 6,h
					H() |=  (1 << 6);
					break;
				case 0xF5: // set 6,l
					L() |=  (1 << 6);
					break;
				case 0xF6: // set 6,(hl)
					t8 = loadB(HL);
					t8 |=  (1 << 6);
					storeB(HL, t8);
					break;
				case 0xF7: // set 6,a
					A() |=  (1 << 6);
					break;
				case 0xF8: // set 7,b
					B() |=  (1 << 7);
					break;
				case 0xF9: // set 7,c
					C() |=  (1 << 7);
					break;
				case 0xFA: // set 7,d
					D() |=  (1 << 7);
					break;
				case 0xFB: // set 7,e
					E() |=  (1 << 7);
					break;
				case 0xFC: // set 7,h
					H() |=  (1 << 7);
					break;
				case 0xFD: // set 7,l
					L() |=  (1 << 7);
					break;
				case 0xFE: // set 7,(hl)
					t8 = loadB(HL);
					t8 |=  (1 << 7);
					storeB(HL, t8);
					break;
				case 0xFF: // set 7,a
					A() |=  (1 << 7);
					break;
			}
			break;
		case 0xCC: // call z,a16
			t16 = loadW();
			if(F() & FLAG_Z) call(t16);
			break;
		case 0xCD: // call a16
			call(loadW());
			break;
		case 0xCE: // adc a,d8
			addA(loadB(), true);
			break;
		case 0xCF: // rst 08h
			call(0x0008);
			break;
		case 0xD0: // ret nc
			if(!(F() & FLAG_C)) ret();
			break;
		case 0xD1: // pop de
			DE = popW();
			break;
		case 0xD2: // jp nc,a16
			t16 = loadW();
			if(!(F() & FLAG_C)) PC = t16;
			break;
		case 0xD3: // out (d8),a
			out(loadB(), A());
			break;
		case 0xD4: // call nc,a16
			t16 = loadW();
			if(!(F() & FLAG_C)) call(t16);
			break;
		case 0xD5: // push de
			pushW(DE);
			break;
		case 0xD6: // sub d8
			subA(loadB());
			break;
		case 0xD7: // rst 10h
			call(0x0010);
			break;
		case 0xD8: // ret c
			if(F() & FLAG_C) ret();
			break;
		case 0xD9: // exx
			swap(BC, BCp);
			swap(DE, DEp);
			swap(HL, HLp);
			break;
		case 0xDA: // jp c,a16
			t16 = loadW();
			if(F() & FLAG_C) PC = t16;
			break;
		case 0xDB: // in a,(d8)
			A() = in(loadB());
			break;
		case 0xDC: // call c,a16
			t16 = loadW();
			if(F() & FLAG_C) call(t16);
			break;
		case 0xDD: // IX
			switch(ins = loadB())
			{
			    case 0x09: // add ix,bc
			        IX += BC;
					break;
				case 0x19: // add ix,de
				    IX += DE;
					break;
				case 0x21: // ld ix,d16
				    IX = loadW();
					break;
				case 0x22: // ld (a16),ix
				    storeW(loadW(), IX);
					break;
				case 0x23: // inc ix
				    ++IX;
					break;
				case 0x29: // add ix,ix
				    IX += IX;
					break;
				case 0x2A: // ld ix,(a16)
				    IX = loadW(loadW());
					break;
				case 0x2B: // dec ix
				    --IX;
					break;
				case 0x34: // inc (ix+s8)
				    t16 = getOff(IX, loadB());
					t8 = loadW(t16) + 1;
					storeB(t16, t8);
					set_inc_flags(t8);
					break;
				case 0x35: // dec (ix+s8)
				    t16 = getOff(IX, loadB());
					t8 = loadW(t16) - 1;
					storeB(t16, t8);
					set_dec_flags(t8);
					break;
				case 0x39: // add ix,sp
				    IX += SP;
					break;
				case 0x46: // ld b,(ix+s8)
				    B() = loadB(getOff(IX, loadB()));
					break;
				case 0x4E: // ld c,(ix+s8)
				    C() = loadB(getOff(IX, loadB()));
					break;
				case 0x56: // ld d,(ix+s8)
				    D() = loadB(getOff(IX, loadB()));
					break;
				case 0x5E: // ld e,(ix+s8)
				    E() = loadB(getOff(IX, loadB()));
					break;
				case 0x66: // ld h,(ix+s8)
				    H() = loadB(getOff(IX, loadB()));
					break;
				case 0x6E: // ld l,(ix+s8)
				    L() = loadB(getOff(IX, loadB()));
					break;
				case 0x70: // ld (ix+s8),b
				    storeB(getOff(IX, loadB()), B());
					break;
				case 0x71: // ld (ix+s8),c
				    storeB(getOff(IX, loadB()), C());
					break;
				case 0x72: // ld (ix+s8),d
				    storeB(getOff(IX, loadB()), D());
					break;
				case 0x73: // ld (ix+s8),e
				    storeB(getOff(IX, loadB()), E());
					break;
				case 0x74: // ld (ix+s8),h
				    storeB(getOff(IX, loadB()), H());
					break;
				case 0x75: // ld (ix+s8),l
				    storeB(getOff(IX, loadB()), L());
					break;
				case 0x77: // ld (ix+s8),a
				    storeB(getOff(IX, loadB()), A());
					break;
				case 0x7E: // ld a,(ix+s8)
				    A() = loadB(getOff(IX, loadB()));
					break;
				case 0x86: // add a,(ix+s8)
					addA(loadB(getOff(IX, loadB())), false);
					break;
				case 0x8E: // adc a,(ix+s8)
					addA(loadB(getOff(IX, loadB())), true);
					break;
				case 0x96: // sub (ix+s8)
					subA(loadB(getOff(IX, loadB())), false);
					break;
				case 0x9E: // sbc a,(ix+s8)
					subA(loadB(getOff(IX, loadB())), true);
					break;
				case 0xA6: // and (ix+s8)
					logicA(A() & loadB(getOff(IX, loadB())), true);
					break;
				case 0xAE: // xor (ix+s8)
					logicA(A() ^ loadB(getOff(IX, loadB())), false);
					break;
				case 0xB6: // or (ix+s8)
					logicA(A() | loadB(getOff(IX, loadB())), false);
					break;
				case 0xBE: // cp (ix+s8)
					t8 = A();
					subA(loadB(getOff(IX, loadB())), false);
					A() = t8;
					break;
				case 0xCB: // IX BITS
					t8 = loadB();
					switch(ins = loadB())
					{
						case 0x06: // rlc (ix+s8)
							t8 = loadB(t16 = getOff(IX, t8));
							rotate_left(t8, BIT_BIT7);
							storeB(t16, t8);
							break;
						case 0x0E: // rrc (ix+s8)
							t8 = loadB(t16 = getOff(IX, t8));
							rotate_right(t8, BIT_BIT7);
							storeB(t16, t8);
							break;
						case 0x16: // rl (ix+s8)
							t8 = loadB(t16 = getOff(IX, t8));
							rotate_left(t8, BIT_CARRY);
							storeB(t16, t8);
							break;
						case 0x1E: // rr (ix+s8)
							t8 = loadB(t16 = getOff(IX, t8));
							rotate_right(t8, BIT_CARRY);
							storeB(t16, t8);
							break;
						case 0x26: // sla (ix+s8)
							t8 = loadB(t16 = getOff(IX, t8));
							rotate_left(t8, BIT_A);
							storeB(t16, t8);
							break;
						case 0x2E: // sra (ix+s8)
							t8 = loadB(t16 = getOff(IX, t8));
							rotate_right(t8, BIT_A);
							storeB(t16, t8);
							break;
						case 0x36: // sll (ix+s8)
							t8 = loadB(t16 = getOff(IX, t8));
							rotate_left(t8, BIT_L);
							storeB(t16, t8);
							break;
						case 0x3E: // srl (ix+s8)
							t8 = loadB(t16 = getOff(IX, t8));
							rotate_right(t8, BIT_L);
							storeB(t16, t8);
							break;
						case 0x46: // bit 0,(ix+s8)
							test_bit(loadB(getOff(IX, t8)), 0);
							break;
						case 0x4E: // bit 1,(ix+s8)
							test_bit(loadB(getOff(IX, t8)), 1);
							break;
						case 0x56: // bit 2,(ix+s8)
							test_bit(loadB(getOff(IX, t8)), 2);
							break;
						case 0x5E: // bit 3,(ix+s8)
							test_bit(loadB(getOff(IX, t8)), 3);
							break;
						case 0x66: // bit 4,(ix+s8)
							test_bit(loadB(getOff(IX, t8)), 4);
							break;
						case 0x6E: // bit 5,(ix+s8)
							test_bit(loadB(getOff(IX, t8)), 5);
							break;
						case 0x76: // bit 6,(ix+s8)
							test_bit(loadB(getOff(IX, t8)), 6);
							break;
						case 0x7E: // bit 7,(ix+s8)
							test_bit(loadB(getOff(IX, t8)), 7);
							break;
						case 0x86: // res 0,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) & ~(1 << 0));
							break;
						case 0x8E: // res 1,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) & ~(1 << 1));
							break;
						case 0x96: // res 2,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) & ~(1 << 2));
							break;
						case 0x9E: // res 3,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) & ~(1 << 3));
							break;
						case 0xA6: // res 4,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) & ~(1 << 4));
							break;
						case 0xAE: // res 5,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) & ~(1 << 5));
							break;
						case 0xB6: // res 6,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) & ~(1 << 6));
							break;
						case 0xBE: // res 7,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) & ~(1 << 7));
							break;
						case 0xC6: // set 0,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) | (1 << 0));
							break;
						case 0xCE: // set 1,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) | (1 << 1));
							break;
						case 0xD6: // set 2,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) | (1 << 2));
							break;
						case 0xDE: // set 3,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) | (1 << 3));
							break;
						case 0xE6: // set 4,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) | (1 << 4));
							break;
						case 0xEE: // set 5,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) | (1 << 5));
							break;
						case 0xF6: // set 6,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) | (1 << 6));
							break;
						case 0xFE: // set 7,(ix+s8)
							t16 = getOff(IX, t8);
							storeB(t16, loadB(t16) | (1 << 7));
							break;
						default:
							throw std::string("ERR: unknown IX BIT instruction!");
					}
					break;
				case 0xE1: // pop ix
				    IX = popW();
					break;
				case 0xE3: // ex (sp),ix
				    SP += 2;
					t16 = popW();
					pushW(IX);
					IX = t16;
					SP -= 2;
					break;
				case 0xE5: // push ix
				    pushW(IX);
					break;
				case 0xE9: // jp (ix)
				    PC = IX;
					break;
				case 0xF9: // ld sp,ix
				    SP = IX;
					break;
			    default:
					throw lib::stringf("Unknown instruction $DD%02X !", ins);
			}
			break;
		case 0xDE: // sbc a,d8
			subA(loadB(), true);
			break;
		case 0xDF: // rst 18h
			call(0x0018);
			break;
		case 0xE0: // ret po
			if(!(F() & FLAG_PV)) ret();
			break;
		case 0xE1: // pop hl
			HL = popW();
			break;
		case 0xE2: // jp po,a16
			t16 = loadW();
			if(!(F() & FLAG_PV)) PC = t16;
			break;
		case 0xE3: // ex (sp),hl
			t16 = loadW(SP);
			storeW(SP, HL);
			HL = t16;
			break;
		case 0xE4: // call po,a16
			t16 = loadW();
			if(!(F() & FLAG_PV)) call(t16);
			break;
		case 0xE5: // push hl
			pushW(HL);
			break;
		case 0xE6: // and d8
			logicA(A() & loadB(), true);
			break;
		case 0xE7: // rst 20h
			call(0x0020);
			break;
		case 0xE8: // ret pe
			if(F() & FLAG_PV) ret();
			break;
		case 0xE9: // jp (hl)
			PC = HL;
			break;
		case 0xEA: // jp pe,a16
			t16 = loadW();
			if(F() & FLAG_PV) PC = t16;
			break;
		case 0xEB: // ex de,hl
			swap(DE, HL);
			break;
		case 0xEC: // call pe,a16
			t16 = loadW();
			if(F() & FLAG_PV) call(t16);
			break;
		case 0xED: // EXTD
			switch(ins = loadB())
			{
			    case 0x40: // in b,(c)
				    set_inc_flags(B() = in(C()));
				    break;
				case 0x41: // out (c),b
				    out(C(), B());
				    break;
				case 0x42: // sbc hl,bc
					subHL(BC, true);
					break;
				case 0x43: // ld (a16),bc
				    storeW(loadW(), BC);
					break;
				case 0x48: // in c,(c)
				    set_inc_flags(C() = in(C()));
				    break;
				case 0x49: // out (c),c
				    out(C(), C());
				    break;
				case 0x4A: // adc hl,bc
					addHL(BC, true);
					break;
			    case 0x4B: // ld bc,(a16)
				    BC = loadW(loadW());
					break;
				case 0x50: // in d,(c)
				    set_inc_flags(D() = in(C()));
				    break;
				case 0x51: // out (c),d
				    out(C(), D());
				    break;
				case 0x52: // sbc hl,de
					subHL(DE, true);
					break;
				case 0x53: // ld (a16),de
				    storeW(loadW(), DE);
					break;
				case 0x56: // im 1
					break;
				case 0x58: // in e,(c)
				    set_inc_flags(E() = in(C()));
				    break;
				case 0x59: // out (c),e
				    out(C(), E());
				    break;
				case 0x5A: // adc hl,de
					addHL(DE, true);
					break;
				case 0x5B: // ld de,(a16)
				    DE = loadW(loadW());
					break;
				case 0x60: // in h,(c)
				    set_inc_flags(H() = in(C()));
					break;
				case 0x61: // out (c),h
				    out(C(), H());
				    break;
				case 0x62: // sbc hl,hl
					subHL(HL, true);
					break;
				case 0x63: // ld (a16),hl
				    storeW(loadW(), HL);
					break;
				case 0x68: // in l,(c)
				    set_inc_flags(L() = in(C()));
				    break;
				case 0x69: // out (c),l
				    out(C(), L());
				    break;
				case 0x6A: // adc hl,hl
					addHL(HL, true);
					break;
				case 0x6B: // ld hl,(a16)
				    HL = loadW(loadW());
					break;
				case 0x70: // in (c)
				    set_inc_flags(in(C()));
				    break;
				case 0x71: // out (c),0
				    out(C(), 0);
				    break;
				case 0x72: // sbc hl,sp
					subHL(SP, true);
					break;
				case 0x73: // ld (a16),sp
				    storeW(loadW(), SP);
					break;
				case 0x78: // in a,(c)
				    set_inc_flags(A() = in(C()));
				    break;
				case 0x79: // out (c),a
				    out(C(), A());
				    break;
				case 0x7A: // adc hl,sp
					addHL(SP, true);
					break;
				case 0x7B: // ld sp,(a16)
				    SP = loadW(loadW());
					break;
				default:
					throw lib::stringf("Unknown instruction $ED%02X !", ins);
			}
			break;
		case 0xEE: // xor d8
			logicA(A() ^ loadB(), false);
			break;
		case 0xEF: // rst 28h
			call(0x0028);
			break;
		case 0xF0: // ret p
			if(!(F() & FLAG_N)) ret();
			break;
		case 0xF1: // pop af
			AF = popW();
			break;
		case 0xF2: // jp p,a16
			t16 = loadW();
			if(!(F() & FLAG_N)) PC = t16;
			break;
		case 0xF3: // di
			int_ = false;
			break;
		case 0xF4: // call p,a16
			t16 = loadW();
			if(!(F() & FLAG_N)) call(t16);
			break;
		case 0xF5: // push af
			pushW(AF);
			break;
		case 0xF6: // or d8
			logicA(A() | loadB(), false);
			break;
		case 0xF7: // rst 30h
			call(0x0030);
			break;
		case 0xF8: // ret m
			if(F() & FLAG_N) ret();
			break;
		case 0xF9: // ld sp,hl
			SP = HL;
			break;
		case 0xFA: // jp m,a16
			t16 = loadW();
			if(F() & FLAG_N) PC = t16;
			break;
		case 0xFB: // ei
			int_ = true;
			break;
		case 0xFC: // call m,a16
			t16 = loadW();
			if(F() & FLAG_N) call(t16);
			break;
		case 0xFD: // IY
			switch(ins = loadB())
			{
			    case 0x09: // add iy,bc
			        IY += BC;
					break;
				case 0x19: // add iy,de
				    IY += DE;
					break;
				case 0x21: // ld iy,d16
				    IY = loadW();
					break;
				case 0x22: // ld (a16),iy
				    storeW(loadW(), IY);
					break;
				case 0x23: // inc iy
				    ++IY;
					break;
				case 0x29: // add iy,iy
				    IY += IY;
					break;
				case 0x2A: // ld iy,(a16)
				    IY = loadW(loadW());
					break;
				case 0x2B: // dec iy
				    --IY;
					break;
				case 0x34: // inc (iy+s8)
				    t16 = getOff(IY, loadB());
					t8 = loadW(t16) + 1;
					storeB(t16, t8);
					set_inc_flags(t8);
					break;
				case 0x35: // dec (iy+s8)
				    t16 = getOff(IY, loadB());
					t8 = loadW(t16) - 1;
					storeB(t16, t8);
					set_dec_flags(t8);
					break;
				case 0x39: // add iy,sp
				    IY += SP;
					break;
				case 0x46: // ld b,(iy+s8)
				    B() = loadB(getOff(IY, loadB()));
					break;
				case 0x4E: // ld c,(iy+s8)
				    C() = loadB(getOff(IY, loadB()));
					break;
				case 0x56: // ld d,(iy+s8)
				    D() = loadB(getOff(IY, loadB()));
					break;
				case 0x5E: // ld e,(iy+s8)
				    E() = loadB(getOff(IY, loadB()));
					break;
				case 0x66: // ld h,(iy+s8)
				    H() = loadB(getOff(IY, loadB()));
					break;
				case 0x6E: // ld l,(iy+s8)
				    L() = loadB(getOff(IY, loadB()));
					break;
				case 0x70: // ld (iy+s8),b
				    storeB(getOff(IY, loadB()), B());
					break;
				case 0x71: // ld (iy+s8),c
				    storeB(getOff(IY, loadB()), C());
					break;
				case 0x72: // ld (iy+s8),d
				    storeB(getOff(IY, loadB()), D());
					break;
				case 0x73: // ld (iy+s8),e
				    storeB(getOff(IY, loadB()), E());
					break;
				case 0x74: // ld (iy+s8),h
				    storeB(getOff(IY, loadB()), H());
					break;
				case 0x75: // ld (iy+s8),l
				    storeB(getOff(IY, loadB()), L());
					break;
				case 0x77: // ld (iy+s8),a
				    storeB(getOff(IY, loadB()), A());
					break;
				case 0x7E: // ld a,(iy+s8)
				    A() = loadB(getOff(IY, loadB()));
					break;
				case 0x86: // add a,(iy+s8)
					addA(loadB(getOff(IY, loadB())), false);
					break;
				case 0x8E: // adc a,(iy+s8)
					addA(loadB(getOff(IY, loadB())), true);
					break;
				case 0x96: // sub (iy+s8)
					subA(loadB(getOff(IY, loadB())), false);
					break;
				case 0x9E: // sbc a,(iy+s8)
					subA(loadB(getOff(IY, loadB())), true);
					break;
				case 0xA6: // and (iy+s8)
					logicA(A() & loadB(getOff(IY, loadB())), true);
					break;
				case 0xAE: // xor (iy+s8)
					logicA(A() ^ loadB(getOff(IY, loadB())), false);
					break;
				case 0xB6: // or (iy+s8)
					logicA(A() | loadB(getOff(IY, loadB())), false);
					break;
				case 0xBE: // cp (iy+s8)
					t8 = A();
					subA(loadB(getOff(IY, loadB())), false);
					A() = t8;
					break;
				case 0xCB: // IY BITS
					t8 = loadB();
					switch(ins = loadB())
					{
						case 0x06: // rlc (iy+s8)
							t8 = loadB(t16 = getOff(IY, t8));
							rotate_left(t8, BIT_BIT7);
							storeB(t16, t8);
							break;
						case 0x0E: // rrc (iy+s8)
							t8 = loadB(t16 = getOff(IY, t8));
							rotate_right(t8, BIT_BIT7);
							storeB(t16, t8);
							break;
						case 0x16: // rl (iy+s8)
							t8 = loadB(t16 = getOff(IY, t8));
							rotate_left(t8, BIT_CARRY);
							storeB(t16, t8);
							break;
						case 0x1E: // rr (iy+s8)
							t8 = loadB(t16 = getOff(IY, t8));
							rotate_right(t8, BIT_CARRY);
							storeB(t16, t8);
							break;
						case 0x26: // sla (iy+s8)
							t8 = loadB(t16 = getOff(IY, t8));
							rotate_left(t8, BIT_A);
							storeB(t16, t8);
							break;
						case 0x2E: // sra (iy+s8)
							t8 = loadB(t16 = getOff(IY, t8));
							rotate_right(t8, BIT_A);
							storeB(t16, t8);
							break;
						case 0x36: // sll (iy+s8)
							t8 = loadB(t16 = getOff(IY, t8));
							rotate_left(t8, BIT_L);
							storeB(t16, t8);
							break;
						case 0x3E: // srl (iy+s8)
							t8 = loadB(t16 = getOff(IY, t8));
							rotate_right(t8, BIT_L);
							storeB(t16, t8);
							break;
						case 0x46: // bit 0,(iy+s8)
							test_bit(loadB(getOff(IY, t8)), 0);
							break;
						case 0x4E: // bit 1,(iy+s8)
							test_bit(loadB(getOff(IY, t8)), 1);
							break;
						case 0x56: // bit 2,(iy+s8)
							test_bit(loadB(getOff(IY, t8)), 2);
							break;
						case 0x5E: // bit 3,(iy+s8)
							test_bit(loadB(getOff(IY, t8)), 3);
							break;
						case 0x66: // bit 4,(iy+s8)
							test_bit(loadB(getOff(IY, t8)), 4);
							break;
						case 0x6E: // bit 5,(iy+s8)
							test_bit(loadB(getOff(IY, t8)), 5);
							break;
						case 0x76: // bit 6,(iy+s8)
							test_bit(loadB(getOff(IY, t8)), 6);
							break;
						case 0x7E: // bit 7,(iy+s8)
							test_bit(loadB(getOff(IY, t8)), 7);
							break;
						case 0x86: // res 0,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) & ~(1 << 0));
							break;
						case 0x8E: // res 1,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) & ~(1 << 1));
							break;
						case 0x96: // res 2,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) & ~(1 << 2));
							break;
						case 0x9E: // res 3,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) & ~(1 << 3));
							break;
						case 0xA6: // res 4,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) & ~(1 << 4));
							break;
						case 0xAE: // res 5,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) & ~(1 << 5));
							break;
						case 0xB6: // res 6,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) & ~(1 << 6));
							break;
						case 0xBE: // res 7,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) & ~(1 << 7));
							break;
						case 0xC6: // set 0,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) | (1 << 0));
							break;
						case 0xCE: // set 1,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) | (1 << 1));
							break;
						case 0xD6: // set 2,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) | (1 << 2));
							break;
						case 0xDE: // set 3,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) | (1 << 3));
							break;
						case 0xE6: // set 4,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) | (1 << 4));
							break;
						case 0xEE: // set 5,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) | (1 << 5));
							break;
						case 0xF6: // set 6,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) | (1 << 6));
							break;
						case 0xFE: // set 7,(iy+s8)
							t16 = getOff(IY, t8);
							storeB(t16, loadB(t16) | (1 << 7));
							break;
						default:
							throw std::string("ERR: unknown IY BIT instruction!");
					}
					break;
				case 0xE1: // pop iy
				    IY = popW();
					break;
				case 0xE3: // ex (sp),iy
				    SP += 2;
					t16 = popW();
					pushW(IY);
					IY = t16;
					SP -= 2;
					break;
				case 0xE5: // push iy
				    pushW(IY);
					break;
				case 0xE9: // jp (iy)
				    PC = IY;
					break;
				case 0xF9: // ld sp,iy
				    SP = IY;
					break;
			    default:
					throw lib::stringf("Unknown instruction $FD%02X !", ins);
			}
			break;
		case 0xFE: // cp d8
			t8 = A();
			subA(loadB());
			A() = t8;
			break;
		case 0xFF: // rst 38h
			call(0x0038);
			break;
		default:
			throw lib::stringf("Unknown instruction $%02X !", ins);
	}
}

bool Z80::parityEven(uint8_t v)
{
	v ^= v >> 4;
	v ^= v >> 2;
	v ^= v >> 1;
	return (~v) & 1;
}

void Z80::addHL(uint16_t v, bool use_c)
{
	uint16_t hl = HL;
	uint16_t cIn, cOut;

	uint cf = (use_c && (F() & FLAG_C)) ? 1 : 0;
	uint16_t t = (HL & 0x0FFF) + (v & 0x0FFF) + cf;

	if(cf)
	{
		cOut = (hl >= 0xFFFF - v) ? 1 : 0;
		HL = hl + v + 1;
	}
	else
	{
		cOut = (hl > 0xFFFF - v) ? 1 : 0;
		HL = hl + v;
	}

	cIn = HL ^ hl ^ v;
	cIn = (cIn >> 15) ^ cOut;

	set_flags(use_c ? FLAG_ALL : FLAG_H | FLAG_N | FLAG_C,
		cIn,
		HL & 0x8000,
		HL == 0,
		t & 0x1000,
		0,
		cOut);
}

void Z80::subHL(uint16_t v, bool use_c)
{
	if(!use_c) F() &= ~FLAG_C;

	F() ^= FLAG_C;
	addHL(~v, true);
	F() ^= FLAG_C;
	F() |= FLAG_N;
}

void Z80::addA(uint8_t v, bool use_c)
{
	uint8_t a = A();
	uint8_t cIn, cOut;

	uint cf = (use_c && (F() & FLAG_C)) ? 1 : 0;
	uint8_t t = (A() & 0x0F) + (v & 0x0F) + cf;

	if(cf)
	{
		cOut = (a >= 0xff - v) ? 1 : 0;
		A() = a + v + 1;
	}
	else
	{
		cOut = (a > 0xff - v) ? 1 : 0;
		A() = a + v;
	}
	
	cIn = A() ^ a ^ v;
	cIn = (cIn >> 7) ^ cOut;
	
	set_flags(FLAG_ALL,
		cIn,
		A() & FLAG_S,
		A() == 0,
		t & FLAG_H,
		0,
		cOut);
}

void Z80::subA(uint8_t v, bool use_c)
{
	if(!use_c) F() &= ~FLAG_C;

	F() ^= FLAG_C;
	addA(~v, true);
	F() ^= FLAG_C;
	F() |= FLAG_N;
}

void Z80::logicA(uint8_t v, bool set_h)
{
	A() = v;
	set_flags(FLAG_ALL,
		parityEven(v),
		v & FLAG_S,
		v == 0,
		set_h,
		0,
		0);
}

void Z80::pushB(uint8_t b)
{
	ram_[--SP] = b;
}

void Z80::pushW(uint16_t bc)
{
	pushB(bc >> 8);
	pushB(bc & 0xff);
}

uint8_t Z80::popB(void)
{
	return ram_[SP++];
}

uint16_t Z80::popW(void)
{
	return popB() | (popB() << 8);
}

void Z80::jr(void)
{
	uint8_t d = ram_[PC++];

	if(d & FLAG_S)
		PC -= (~d & 0xff) + 1;
	else
		PC += d;
}

void Z80::out(uint8_t port, uint8_t data)
{
	auto p = periphs_.find(port >> 4);

	if(p != periphs_.end())
	{
		p->second->write(port & 0x0F, data);
	}
}

uint8_t Z80::in(uint8_t port)
{
	auto p = periphs_.find(port >> 4);

	if(p != periphs_.end())
	{
		return p->second->read(port & 0x0F);
	}

	return 0;
}

uint8_t Z80::loadB(void)
{
	return loadB(PC++);
}

uint16_t Z80::loadW(void)
{
	uint16_t r = loadW(PC);

	PC += 2;

	return r;
}

uint8_t Z80::loadB(uint16_t a)
{
	return ram_[a];
}

uint16_t Z80::loadW(uint16_t a)
{
	return loadB(a) | (loadB(a + 1) << 8);
}

void Z80::storeB(uint16_t a, uint8_t v)
{
	ram_[a] = v;
}

void Z80::storeW(uint16_t a, uint16_t v)
{
	storeB(a + 0, v & 0xFF);
	storeB(a + 1, v >> 8);
}

void Z80::call(uint16_t a)
{
	pushW(PC);
	PC = a;
}

void Z80::ret(void)
{
	PC = popW();
}

void Z80::rotate_left(uint8_t& r, uint bit0)
{
	bool c = r & FLAG_S;
	r <<= 1;
	switch(bit0)
	{
		case BIT_BIT7: r |= c ? 1 : 0; break;
		case BIT_CARRY: r |= F() & FLAG_C ? 1 : 0; break;
		case BIT_L: r |= 1; break;
	}
	set_flags(FLAG_ALL, parityEven(r), r & FLAG_S, r == 0, 0, 0, c);
}

void Z80::rotate_right(uint8_t& r, uint bit7)
{
	bool c = r & 1;
	r >>= 1;
	switch(bit7)
	{
		case BIT_BIT7: r |= c ? 0x80 : 0; break;
		case BIT_CARRY: r |= F() & FLAG_C ? 0x80 : 0; break;
		case BIT_A: r |= r & 0x40 ? 0x80 : 0; break;
	}
	set_flags(FLAG_ALL, parityEven(r), r & FLAG_S, r == 0, 0, 0, c);
}

void Z80::rlca(void)
{
	F() = (A() & FLAG_S) ? (F() | FLAG_C) : (F() & ~FLAG_C);
	A() <<= 1;
	if(F() & FLAG_C) A() |= 1;
	F() &= ~FLAG_H & ~FLAG_N;
}

void Z80::rla(void)
{
	uint t = (F() & FLAG_C) ? 1 : 0;
	F() = (A() & FLAG_S) ? (F() | FLAG_C) : (F() & ~FLAG_C);
	A() <<= 1;
	A() |= t;
	F() &= ~FLAG_H & ~FLAG_N;
}

void Z80::rrca(void)
{
	F() = (A() & 1) ? (F() | FLAG_C) : (F() & ~FLAG_C);
	A() >>= 1;
	if(F() & FLAG_C) A() |= 0x80;
	F() &= ~FLAG_H & ~FLAG_N;
}

void Z80::rra(void)
{
	uint t = (F() & FLAG_C) ? 0x80 : 0;
	F() = (A() & 1) ? (F() | FLAG_C) : (F() & ~FLAG_C);
	A() >>= 1;
	A() |= t;
	F() &= ~FLAG_H & ~FLAG_N;
}

void Z80::daa(void)
{
	if((A() & 0x0F) > 9 || (F() & FLAG_H))
	{
		A() += 0x06;
		F() |= FLAG_H;
	}
	else
	{
		F() &= FLAG_H;
	}

	if((A() >> 4) > 9 || (F() & FLAG_C))
	{
		A() += 0x60;
		F() |= FLAG_C;
	}
	else
	{
		F() &= ~FLAG_C;
	}

	F() = parityEven(A()) ? (F() | FLAG_PV) : (F() & ~FLAG_PV);
}

void Z80::set_inc_flags(uint8_t v)
{
	set_flags(FLAG_ALL & ~FLAG_C,
		v == 0x80,	   // P/V
		v & FLAG_S,	  // S
		v == 0,		  // Z
		(v & 0x0F) == 0, // H
		0,			   // N
		0);			  // C
}

void Z80::set_dec_flags(uint8_t v)
{
	set_flags(FLAG_ALL & ~FLAG_C,
		v == 0x7F,		  // P/V
		v & FLAG_S,		 // S
		v == 0,			 // Z
		(v & 0x0F) == 0x0F, // H
		1,				  // N
		0);				 // C
}

uint16_t Z80::getOff(uint16_t a, uint8_t v)
{
	return a + (((v & FLAG_S) ? 0xFF00 : 0x0000) | v);
}

void Z80::set_flags(uint flags, uint f_pv, uint f_s, uint f_z, uint f_h, uint f_n, uint f_c)
{
	if(flags & FLAG_PV)
	{
		F() = f_pv ? (F() | FLAG_PV) : (F() & ~FLAG_PV);
	}

	if(flags & FLAG_S)
	{
		F() = f_s ? (F() | FLAG_S) : (F() & ~FLAG_S);
	}

	if(flags & FLAG_Z)
	{
		F() = f_z ? (F() | FLAG_Z) : (F() & ~FLAG_Z);
	}

	if(flags & FLAG_H)
	{
		F() = f_h ? (F() | FLAG_H) : (F() & ~FLAG_H);
	}

	if(flags & FLAG_N)
	{
		F() = f_n ? (F() | FLAG_N) : (F() & ~FLAG_N);
	}

	if(flags & FLAG_C)
	{
		F() = f_c ? (F() | FLAG_C) : (F() & ~FLAG_C);
	}
}

std::string Z80::disassemble(uint16_t addr) const
{
	return z80::disassemble(&ram_[addr]).literal;
}

void Z80::clear(void)
{
	AF = AFp = BC = BCp = DE = DEp = HL = HLp = IR = IX = IY = SP = PC = 0;
	for(uint i = 0 ; i < 0x10000 ; ++i)
	{
		ram_[i] = 0;
	}
}

}


