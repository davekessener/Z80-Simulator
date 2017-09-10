#ifndef Z80_STATUSPORT_H
#define Z80_STATUSPORT_H

#include <deque>

#include "Peripheral.h"
#include "Property.h"

namespace z80
{
	class StatusPort : public Peripheral
	{
		public:
		typedef lib::Property<bool> int_t;
		typedef std::function<void(void)> int_fn;

		static const uint8_t STATUS_ID = 0x3A;

		public:
			void write(uint8_t, uint8_t);
			uint8_t read(uint8_t);
			void registerInt(uint8_t, int_t);
			void onInt(int_fn f) { onInt_ = f; }
			void reset( );
		private:
			std::deque<uint8_t> queue_;
			int_fn onInt_;
			uint idRead_ = 0;
	};
}

#endif

