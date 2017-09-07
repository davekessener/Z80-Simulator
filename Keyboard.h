#ifndef Z80_KEYBOARD_H
#define Z80_KEYBOARD_H

#include <map>
#include <deque>

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
			Keyboard( );
			void press(uint, bool);
			int_t keyPressedInt( ) { return intLine_; }
			void write(uint8_t, uint8_t);
			uint8_t read(uint8_t);
		private:
			static void init( );

		private:
			int_t intLine_;
			std::deque<uint8_t> buf_;

			static std::map<uint, uint8_t> mSimple;
			static std::map<uint, uint8_t> mExtended;
	};
}

#endif

