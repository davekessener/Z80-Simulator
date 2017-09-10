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

		static const uint MODE_POLL = 0x00;
		static const uint MODE_RAW  = 0x01;
		static const uint MODE_TEXT = 0x02;

		public:
			Keyboard( );
			void press(uint, bool);
			int_t keyPressedInt( ) { return intLine_; }
			void write(uint8_t, uint8_t);
			uint8_t read(uint8_t);
			void reset( );
		private:
			uint8_t getASCII(uint) const;
			static void init( );

		private:
			int_t intLine_;
			std::deque<uint8_t> buf_;
			std::vector<uint8_t> pressed_;
			uint mode_;
			uint8_t poll_;
			bool shift_, ctrl_, alt_;

			static std::map<uint, uint8_t> mSimple;
			static std::map<uint, uint8_t> mExtended;
			static std::map<uint, uint8_t> mASCII;
			static std::map<uint, uint8_t> mASCIIshifted;
	};
}

#endif

