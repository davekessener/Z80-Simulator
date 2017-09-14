#ifndef Z80_STATUSWINDOW_H
#define Z80_STATUSWINDOW_H

#include "Z80.h"
#include "CharacterWindow.h"
#include "Image.h"

namespace z80
{
	class StatusWindow : public winui::CharacterWindow
	{
		public:
			StatusWindow(Z80&);
		private:
			void onUpdate(uint);
			void onRender( );
			void onEvent(const SDL_Event&);

		private:
			Z80 *cpu_;
	};
}

#endif

