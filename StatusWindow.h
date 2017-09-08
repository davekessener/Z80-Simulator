#ifndef Z80_STATUSWINDOW_H
#define Z80_STATUSWINDOW_H

#include "Z80.h"
#include "Window.h"
#include "Image.h"

namespace z80
{
	class StatusWindow
	{
		public:
			StatusWindow(Z80&);
			void setWindowIcon(winui::Image i) { window_.setWindowIcon(i); }
			void show( ) { window_.show(); }
			void hide( ) { window_.hide(); }
		private:
			void onUpdate(uint);
			void onRender( );
			void onEvent(const SDL_Event&);

		private:
			winui::Window window_;
			winui::Image charset_;
			Z80 *cpu_;
	};
}

#endif

