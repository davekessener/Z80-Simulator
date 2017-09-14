#ifndef Z80_SCREENWINDOW_H
#define Z80_SCREENWINDOW_H

#include "CharacterWindow.h"
#include "Image.h"
#include "Screen.h"
#include "Keyboard.h"
#include "Property.h"

namespace z80
{
	class ScreenWindow : public winui::CharacterWindow
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

		private:
			Screen *screen_;
			Keyboard *keyboard_;
			uint fps_;
			int_t int_;
	};
}

#endif

