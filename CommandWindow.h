#ifndef LIB_WINUI_COMMANDWINDOW_H
#define LIB_WINUI_COMMANDWINDOW_H

#include <functional>
#include <string>
#include <vector>
#include <deque>

#include <SDL.h>

#include "CharacterWindow.h"
#include "Image.h"

namespace winui
{
	class CommandWindow : public CharacterWindow
	{
		public:
		typedef std::function<void(const std::string&)> exec_fn;
		typedef std::function<void(void)> close_fn;

		public:
			CommandWindow(const std::string&, const Dimension&, Image, const Dimension&, uint);
			void setPrompt(const std::string& s) { prompt_ = s; }
			void setExecutionHandler(exec_fn f) { run_ = f; }
			void setCloseHandler(close_fn f) { onClose_ = f; }
			void println(const std::string&);
			void setFontColor(uint c) { setFontColorIndex(c); }
		private:
			void onRender( );
			void onUpdate(uint);
			void onEvent(const SDL_Event&);
			void input(char);

		private:
			exec_fn run_;
			close_fn onClose_;
			std::string prompt_;

			std::vector<std::string> buffer_;
			std::deque<std::string> prev_;
			std::string cmd_, tmp_;
			uint offBuf_, offCmd_, offPrev_;
	};
}

#endif

