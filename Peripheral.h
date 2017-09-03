#ifndef Z80_PERIPHERAL_H
#define Z80_PERIPHERAL_H

#include <stdint.h>

typedef unsigned uint;

namespace z80
{
	class Peripheral
	{
		public:
			static const uint8_t PER_ID = 0xDA;

		public:
			virtual void write(uint8_t, uint8_t) = 0;
			virtual uint8_t read(uint8_t) = 0;
	};
}

#endif

