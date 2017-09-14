#ifndef WINUI_CHARACTERWINDOW_H
#define WINUI_CHARACTERWINDOW_H

#include "Window.h"
#include "Image.h"

namespace winui
{
	class CharacterWindow : public Window
	{
		public:
			CharacterWindow(const std::string&, Position, Dimension, Image, Dimension, uint);
		protected:
			void onUpdate(uint);
			void setFontColorIndex(uint i) { if(i < colorspace_) color_ = i; }
			void setBlinkSpeed(uint v) { blinkspeed_ = v; }
			void enableBlink(bool v) { en_blink_ = v; }
			void blinkIndependently(bool v) { blinkIndependently_ = v; }
			bool isBlinkEnabled( ) const { return en_blink_; }
			bool isBlinking( ) const
				{ return (hasFocus() || blinkIndependently_) && en_blink_ && blink_ <= blinkspeed_ / 2; }
			Dimension getSize( ) const { return winsize_; }
			void updateCursor(Position p) { cursor_ = p; }
			void renderChar(Position, uint8_t, uint = 0, bool = false);
			void renderString(Position, const std::string&);
			void renderCursor( ) { renderChar(cursor_, ' ', color_, false); }

		private:
			Image charset_;
			uint colorspace_;
			Dimension winsize_, charsize_;
			uint blink_, blinkspeed_;
			Position cursor_;
			bool en_blink_, blinkIndependently_;
			uint color_;
	};
}

#endif

