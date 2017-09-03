#ifndef Z80_SCREEN_H
#define Z80_SCREEN_H

#include "Peripheral.h"
#include "Property.h"

namespace z80
{
	class Screen : public Peripheral
	{
		public:
			static const uint32_t COLS = 80;
			static const uint32_t ROWS = 60;
			static const uint8_t SCREEN_ID = 0x2C;

		public:
			Screen( );
			~Screen( );
			void write(uint8_t, uint8_t);
			uint8_t read(uint8_t);
			bool cursor_en()	{ return status_ & (1 << 0); }
			bool advX()			{ return status_ & (1 << 1); }
			bool wrapX()		{ return status_ & (1 << 2); }
			bool advY()			{ return status_ & (1 << 3); }
			bool scroll_en()	{ return status_ & (1 << 4); }
			bool inverted()		{ return status_ & (1 << 5); }
			bool screen_en()	{ return status_ & (1 << 6); }
			bool timer_en()		{ return timerEn_; }
			uint8_t getCursorX( ) const { return cx_; }
			uint8_t getCursorY( ) const { return cy_; }
			uint8_t getChar(uint x, uint y) const { return vram_[x + y * COLS]; }
		private:
			void command(uint8_t);
			void do_scroll( );
			void writeToVRAM(uint8_t);

		private:
			uint8_t vram_[COLS*ROWS];
			uint8_t cx_, cy_;
			uint8_t status_;
			uint8_t id_ = 0;
			bool timerEn_;
	};
}

#endif

