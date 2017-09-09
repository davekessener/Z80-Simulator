#ifndef Z80_DISASSEMBLE_H
#define Z80_DISASSEMBLE_H

#include <string>

#include "lib.h"

namespace z80
{
	struct Instruction
	{
		std::string literal;
		const uint8_t *next;
	};

	Instruction disassemble(const uint8_t *);
}

#endif

