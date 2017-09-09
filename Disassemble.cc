#include "Disassemble.h"

namespace z80 {

Instruction disassemble(const uint8_t *ins)
{
	const uint8_t *p = ins;

	using lib::stringf;

	auto a = [&p](const std::string& v) -> Instruction
	{
		Instruction i;

		i.literal = v;
		i.next = p;

		return i;
	};

#define WORD ((uint16_t *) (p += 2))[-1]
#define BYTE *p++
#define OFF (int)((char) BYTE)
	switch(*p++)
	{
		case 0x00: return a(stringf("nop"));
		case 0x01: return a(stringf("ld bc,$%04X", WORD));
		case 0x02: return a(stringf("ld (bc),a"));
		case 0x03: return a(stringf("inc bc"));
		case 0x04: return a(stringf("inc b"));
		case 0x05: return a(stringf("dec b"));
		case 0x06: return a(stringf("ld b,0x%02X", BYTE));
		case 0x07: return a(stringf("rlca"));
		case 0x08: return a(stringf("ex af,af'"));
		case 0x09: return a(stringf("add hl,bc"));
		case 0x0a: return a(stringf("ld a,(bc)"));
		case 0x0b: return a(stringf("dec bc"));
		case 0x0c: return a(stringf("inc c"));
		case 0x0d: return a(stringf("dec c"));
		case 0x0e: return a(stringf("ld c,0x%02X", BYTE));
		case 0x0f: return a(stringf("rrca"));
		case 0x10: return a(stringf("djnz %d", OFF));
		case 0x11: return a(stringf("ld de,$%04X", WORD));
		case 0x12: return a(stringf("ld (de),a"));
		case 0x13: return a(stringf("inc de"));
		case 0x14: return a(stringf("inc d"));
		case 0x15: return a(stringf("dec d"));
		case 0x16: return a(stringf("ld d,0x%02X", BYTE));
		case 0x17: return a(stringf("rla"));
		case 0x18: return a(stringf("jr %d", OFF));
		case 0x19: return a(stringf("add hl,de"));
		case 0x1a: return a(stringf("ld a,(de)"));
		case 0x1b: return a(stringf("dec de"));
		case 0x1c: return a(stringf("inc e"));
		case 0x1d: return a(stringf("dec e"));
		case 0x1e: return a(stringf("ld e,0x%02X", BYTE));
		case 0x1f: return a(stringf("rra"));
		case 0x20: return a(stringf("jr nz,%d", OFF));
		case 0x21: return a(stringf("ld hl,$%04X", WORD));
		case 0x22: return a(stringf("ld ($%04X),hl", WORD));
		case 0x23: return a(stringf("inc hl"));
		case 0x24: return a(stringf("inc h"));
		case 0x25: return a(stringf("dec h"));
		case 0x26: return a(stringf("ld h,0x%02X", BYTE));
		case 0x27: return a(stringf("daa"));
		case 0x28: return a(stringf("jr z,%d", OFF));
		case 0x29: return a(stringf("add hl,hl"));
		case 0x2a: return a(stringf("ld hl,($%04X)", WORD));
		case 0x2b: return a(stringf("dec hl"));
		case 0x2c: return a(stringf("inc l"));
		case 0x2d: return a(stringf("dec l"));
		case 0x2e: return a(stringf("ld l,0x%02X", BYTE));
		case 0x2f: return a(stringf("cpl"));
		case 0x30: return a(stringf("jr nc,%d", OFF));
		case 0x31: return a(stringf("ld sp,$%04X", WORD));
		case 0x32: return a(stringf("ld ($%04X),a", WORD));
		case 0x33: return a(stringf("inc sp"));
		case 0x34: return a(stringf("inc (hl)"));
		case 0x35: return a(stringf("dec (hl)"));
		case 0x36: return a(stringf("ld (hl),0x%02X", BYTE));
		case 0x37: return a(stringf("scf"));
		case 0x38: return a(stringf("jr c,%d", OFF));
		case 0x39: return a(stringf("add hl,sp"));
		case 0x3a: return a(stringf("ld a,($%04X)", WORD));
		case 0x3b: return a(stringf("dec sp"));
		case 0x3c: return a(stringf("inc a"));
		case 0x3d: return a(stringf("dec a"));
		case 0x3e: return a(stringf("ld a,0x%02X", BYTE));
		case 0x3f: return a(stringf("ccf"));
		case 0x40: return a(stringf("ld b,b"));
		case 0x41: return a(stringf("ld b,c"));
		case 0x42: return a(stringf("ld b,d"));
		case 0x43: return a(stringf("ld b,e"));
		case 0x44: return a(stringf("ld b,h"));
		case 0x45: return a(stringf("ld b,l"));
		case 0x46: return a(stringf("ld b,(hl)"));
		case 0x47: return a(stringf("ld b,a"));
		case 0x48: return a(stringf("ld c,b"));
		case 0x49: return a(stringf("ld c,c"));
		case 0x4a: return a(stringf("ld c,d"));
		case 0x4b: return a(stringf("ld c,e"));
		case 0x4c: return a(stringf("ld c,h"));
		case 0x4d: return a(stringf("ld c,l"));
		case 0x4e: return a(stringf("ld c,(hl)"));
		case 0x4f: return a(stringf("ld c,a"));
		case 0x50: return a(stringf("ld d,b"));
		case 0x51: return a(stringf("ld d,c"));
		case 0x52: return a(stringf("ld d,d"));
		case 0x53: return a(stringf("ld d,e"));
		case 0x54: return a(stringf("ld d,h"));
		case 0x55: return a(stringf("ld d,l"));
		case 0x56: return a(stringf("ld d,(hl)"));
		case 0x57: return a(stringf("ld d,a"));
		case 0x58: return a(stringf("ld e,b"));
		case 0x59: return a(stringf("ld e,c"));
		case 0x5a: return a(stringf("ld e,d"));
		case 0x5b: return a(stringf("ld e,e"));
		case 0x5c: return a(stringf("ld e,h"));
		case 0x5d: return a(stringf("ld e,l"));
		case 0x5e: return a(stringf("ld e,(hl)"));
		case 0x5f: return a(stringf("ld e,a"));
		case 0x60: return a(stringf("ld h,b"));
		case 0x61: return a(stringf("ld h,c"));
		case 0x62: return a(stringf("ld h,d"));
		case 0x63: return a(stringf("ld h,e"));
		case 0x64: return a(stringf("ld h,h"));
		case 0x65: return a(stringf("ld h,l"));
		case 0x66: return a(stringf("ld h,(hl)"));
		case 0x67: return a(stringf("ld h,a"));
		case 0x68: return a(stringf("ld l,b"));
		case 0x69: return a(stringf("ld l,c"));
		case 0x6a: return a(stringf("ld l,d"));
		case 0x6b: return a(stringf("ld l,e"));
		case 0x6c: return a(stringf("ld l,h"));
		case 0x6d: return a(stringf("ld l,l"));
		case 0x6e: return a(stringf("ld l,(hl)"));
		case 0x6f: return a(stringf("ld l,a"));
		case 0x70: return a(stringf("ld (hl),b"));
		case 0x71: return a(stringf("ld (hl),c"));
		case 0x72: return a(stringf("ld (hl),d"));
		case 0x73: return a(stringf("ld (hl),e"));
		case 0x74: return a(stringf("ld (hl),h"));
		case 0x75: return a(stringf("ld (hl),l"));
		case 0x76: return a(stringf("halt"));
		case 0x77: return a(stringf("ld (hl),a"));
		case 0x78: return a(stringf("ld a,b"));
		case 0x79: return a(stringf("ld a,c"));
		case 0x7a: return a(stringf("ld a,d"));
		case 0x7b: return a(stringf("ld a,e"));
		case 0x7c: return a(stringf("ld a,h"));
		case 0x7d: return a(stringf("ld a,l"));
		case 0x7e: return a(stringf("ld a,(hl)"));
		case 0x7f: return a(stringf("ld a,a"));
		case 0x80: return a(stringf("add a,b"));
		case 0x81: return a(stringf("add a,c"));
		case 0x82: return a(stringf("add a,d"));
		case 0x83: return a(stringf("add a,e"));
		case 0x84: return a(stringf("add a,h"));
		case 0x85: return a(stringf("add a,l"));
		case 0x86: return a(stringf("add a,(hl)"));
		case 0x87: return a(stringf("add a,a"));
		case 0x88: return a(stringf("adc a,b"));
		case 0x89: return a(stringf("adc a,c"));
		case 0x8a: return a(stringf("adc a,d"));
		case 0x8b: return a(stringf("adc a,e"));
		case 0x8c: return a(stringf("adc a,h"));
		case 0x8d: return a(stringf("adc a,l"));
		case 0x8e: return a(stringf("adc a,(hl)"));
		case 0x8f: return a(stringf("adc a,a"));
		case 0x90: return a(stringf("sub b"));
		case 0x91: return a(stringf("sub c"));
		case 0x92: return a(stringf("sub d"));
		case 0x93: return a(stringf("sub e"));
		case 0x94: return a(stringf("sub h"));
		case 0x95: return a(stringf("sub l"));
		case 0x96: return a(stringf("sub (hl)"));
		case 0x97: return a(stringf("sub a"));
		case 0x98: return a(stringf("sbc a,b"));
		case 0x99: return a(stringf("sbc a,c"));
		case 0x9a: return a(stringf("sbc a,d"));
		case 0x9b: return a(stringf("sbc a,e"));
		case 0x9c: return a(stringf("sbc a,h"));
		case 0x9d: return a(stringf("sbc a,l"));
		case 0x9e: return a(stringf("sbc a,(hl)"));
		case 0x9f: return a(stringf("sbc a,a"));
		case 0xa0: return a(stringf("and b"));
		case 0xa1: return a(stringf("and c"));
		case 0xa2: return a(stringf("and d"));
		case 0xa3: return a(stringf("and e"));
		case 0xa4: return a(stringf("and h"));
		case 0xa5: return a(stringf("and l"));
		case 0xa6: return a(stringf("and (hl)"));
		case 0xa7: return a(stringf("and a"));
		case 0xa8: return a(stringf("xor b"));
		case 0xa9: return a(stringf("xor c"));
		case 0xaa: return a(stringf("xor d"));
		case 0xab: return a(stringf("xor e"));
		case 0xac: return a(stringf("xor h"));
		case 0xad: return a(stringf("xor l"));
		case 0xae: return a(stringf("xor (hl)"));
		case 0xaf: return a(stringf("xor a"));
		case 0xb0: return a(stringf("or b"));
		case 0xb1: return a(stringf("or c"));
		case 0xb2: return a(stringf("or d"));
		case 0xb3: return a(stringf("or e"));
		case 0xb4: return a(stringf("or h"));
		case 0xb5: return a(stringf("or l"));
		case 0xb6: return a(stringf("or (hl)"));
		case 0xb7: return a(stringf("or a"));
		case 0xb8: return a(stringf("cp b"));
		case 0xb9: return a(stringf("cp c"));
		case 0xba: return a(stringf("cp d"));
		case 0xbb: return a(stringf("cp e"));
		case 0xbc: return a(stringf("cp h"));
		case 0xbd: return a(stringf("cp l"));
		case 0xbe: return a(stringf("cp (hl)"));
		case 0xbf: return a(stringf("cp a"));
		case 0xc0: return a(stringf("ret nz"));
		case 0xc1: return a(stringf("pop bc"));
		case 0xc2: return a(stringf("jp nz,$%04X", WORD));
		case 0xc3: return a(stringf("jp $%04X", WORD));
		case 0xc4: return a(stringf("call nz,$%04X", WORD));
		case 0xc5: return a(stringf("push bc"));
		case 0xc6: return a(stringf("add a,0x%02X", BYTE));
		case 0xc7: return a(stringf("rst 00h"));
		case 0xc8: return a(stringf("ret z"));
		case 0xc9: return a(stringf("ret"));
		case 0xca: return a(stringf("jp z,$%04X", WORD));
		case 0xcb: switch(BYTE)
		{
			default:
				return a("BITS");
		}
		case 0xcc: return a(stringf("call z,$%04X", WORD));
		case 0xcd: return a(stringf("call $%04X", WORD));
		case 0xce: return a(stringf("adc a,0x%02X", BYTE));
		case 0xcf: return a(stringf("rst 08h"));
		case 0xd0: return a(stringf("ret nc"));
		case 0xd1: return a(stringf("pop de"));
		case 0xd2: return a(stringf("jp nc,$%04X", WORD));
		case 0xd3: return a(stringf("out (0x%02X),a", BYTE));
		case 0xd4: return a(stringf("call nc,$%04X", WORD));
		case 0xd5: return a(stringf("push de"));
		case 0xd6: return a(stringf("sub 0x%02X", BYTE));
		case 0xd7: return a(stringf("rst 10h"));
		case 0xd8: return a(stringf("ret c"));
		case 0xd9: return a(stringf("exx"));
		case 0xda: return a(stringf("jp c,$%04X", WORD));
		case 0xdb: return a(stringf("in a,(0x%02X)", BYTE));
		case 0xdc: return a(stringf("call c,$%04X", WORD));
		case 0xdd: switch(BYTE)
		{
			default:
				return a("IX");
		}
		case 0xde: return a(stringf("sbc a,0x%02X", BYTE));
		case 0xdf: return a(stringf("rst 18h"));
		case 0xe0: return a(stringf("ret po"));
		case 0xe1: return a(stringf("pop hl"));
		case 0xe2: return a(stringf("jp po,$%04X", WORD));
		case 0xe3: return a(stringf("ex (sp),hl"));
		case 0xe4: return a(stringf("call po,$%04X", WORD));
		case 0xe5: return a(stringf("push hl"));
		case 0xe6: return a(stringf("and 0x%02X", BYTE));
		case 0xe7: return a(stringf("rst 20h"));
		case 0xe8: return a(stringf("ret pe"));
		case 0xe9: return a(stringf("jp (hl)"));
		case 0xea: return a(stringf("jp pe,$%04X", WORD));
		case 0xeb: return a(stringf("ex de,hl"));
		case 0xec: return a(stringf("call pe,$%04X", WORD));
		case 0xed: switch(BYTE)
		{
			default:
				return a("EXTD");
		}
		case 0xee: return a(stringf("xor 0x%02X", BYTE));
		case 0xef: return a(stringf("rst 28h"));
		case 0xf0: return a(stringf("ret p"));
		case 0xf1: return a(stringf("pop af"));
		case 0xf2: return a(stringf("jp p,$%04X", WORD));
		case 0xf3: return a(stringf("di"));
		case 0xf4: return a(stringf("call p,$%04X", WORD));
		case 0xf5: return a(stringf("push af"));
		case 0xf6: return a(stringf("or 0x%02X", BYTE));
		case 0xf7: return a(stringf("rst 30h"));
		case 0xf8: return a(stringf("ret m"));
		case 0xf9: return a(stringf("ld sp,hl"));
		case 0xfa: return a(stringf("jp m,$%04X", WORD));
		case 0xfb: return a(stringf("ei"));
		case 0xfc: return a(stringf("call m,$%04X", WORD));
		case 0xfd: switch(BYTE)
		{
			default:
				return a("IY");
		}
		case 0xfe: return a(stringf("cp 0x%02X", BYTE));
		case 0xff: return a(stringf("rst 38h"));
	}
#undef WORD
#undef BYTE
#undef OFF
}

}
