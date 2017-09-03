#ifndef Z80_SCREENWINDOW_H
#define Z80_SCREENWINDOW_H

#include "Window.h"
#include "Image.h"
#include "Screen.h"
#include "Keyboard.h"
#include "Property.h"

#define MXT_SCREEN_TITLE "Z80 Screen Simulator"
#define MXT_CHARW 8
#define MXT_CHARH 8
#define MXT_SCREEN_COLS 80
#define MXT_SCREEN_ROWS 60
#define MXT_SCREEN_WIDTH (MXT_SCREEN_COLS*MXT_CHARW)
#define MXT_SCREEN_HEIGHT (MXT_SCREEN_ROWS*MXT_CHARH)
#define MXT_CHARSET "dascii.bmp"

#define MXT_BLINK 1000

namespace z80
{
	class ScreenWindow
	{
		public:
		typedef lib::Property<bool> int_t;

		public:
			ScreenWindow(Screen&, Keyboard&);
			int_t int60fps( ) { return int_; }
		private:
			void onUpdate(uint);
			void onRender( );
			void onEvent(const SDL_Event&);
			bool isBlinking( ) const { return blink_ >= MXT_BLINK / 2; }
			void drawChar(uint, uint, char, bool);

		private:
			winui::Window window_;
			winui::Image charset_;
			Screen *screen_;
			Keyboard *keyboard_;
			uint blink_, fps_;
			int_t int_;
	};
}

#endif

