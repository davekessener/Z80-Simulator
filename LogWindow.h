#ifndef Z80_LOGWINDOW_H
#define Z80_LOGWINDOW_H

#include <vector>

#include "CharacterWindow.h"

namespace z80
{
	class LogWindow : public winui::CharacterWindow
	{
		public:
			LogWindow( );
			void println(const std::string&);
		protected:
			void onRender( );
			void onEvent(const SDL_Event&);
		private:
			void scroll(int);

		private:
			std::vector<std::string> lines_;
			uint off_;
	};
}

#endif

