#ifndef LIB_WINUI_COMMANDWINDOW_H
#define LIB_WINUI_COMMANDWINDOW_H

#include <string>
#include <vector>
#include <deque>

#include <SDL.h>

#include "Window.h"
#include "Image.h"

namespace winui
{
	class CommandWindow
	{
		public:
		typedef std::function<void(const std::string&)> exec_fn;

		public:
			CommandWindow(const std::string&, const Dimension&, Image, const Dimension&, uint);
			void setPrompt(const std::string& s) { prompt_ = s; }
			void setExecutionHandler(exec_fn f) { run_ = f; }
			void setBackgroundColor(const Color& c) { window_.setDefaultColor(c); }
			void setFontColorIndex(uint i) { if(i < nColors_) color_ = i; }
			void setBlinkSpeed(uint s) { blinkSpeed_ = s; blink_ = 0; }
			void println(const std::string&);
		private:
			void onRender( );
			void onUpdate(uint);
			void onEvent(const SDL_Event&);
			void renderChar(uint, uint, char, bool);
			void renderString(uint, const std::string&);
			bool isBlinking( ) const { return window_.hasFocus() && (blink_ >= blinkSpeed_ / 2); }
			void input(char);

		private:
			Window window_;
			Image charset_;
			exec_fn run_;
			std::string prompt_;
			uint color_, nColors_;
			Dimension screen_, charsize_;
			Position cursor_;
			uint blink_, blinkSpeed_;

			std::vector<std::string> buffer_;
			std::deque<std::string> prev_;
			std::string cmd_, tmp_;
			uint offBuf_, offCmd_, offPrev_;
	};
}

#endif

