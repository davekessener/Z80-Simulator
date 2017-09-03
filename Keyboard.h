#ifndef Z80_KEYBOARD_H
#define Z80_KEYBOARD_H

#include "WinUI.h"
#include "Peripheral.h"
#include "Property.h"

namespace z80
{
	class Keyboard : public Peripheral
	{
		public:
		typedef lib::Property<bool> int_t;

		public:
			Keyboard( ) : int_(new bool) { int_.set(false); }
			void press(uint);
			int_t keyPressedInt( ) { return int_; }
			void write(uint8_t, uint8_t);
			uint8_t read(uint8_t);
		private:
			int_t int_;
	};
}

#endif

