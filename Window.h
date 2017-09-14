#ifndef LIB_SDL_WINDOW_H
#define LIB_SDL_WINDOW_H

#include <string>

#include <SDL.h>

#include "WinUI.h"
#include "Image.h"

#include "Timer.h"

namespace winui
{
	class Window
	{
		public:
			Window(const std::string&, const Space&);
			virtual ~Window( );
			void hide( );
			void show( );
			bool isHidden( ) const { return hidden_; }
			bool isMouseOver( ) const { return mouseOver_; }
			bool hasFocus( ) const { return focus_; }
			uint getID( ) const { return windowID_; }
			void update( );
			void render( );
			void clear( );
			void setDefaultColor(const Color& c) { default_ = c; }
			void setWindowIcon(Image);
			void draw(Image img, const Position& p)
				{ draw(img, Space(p.x, p.y, img.getWidth(), img.getHeight())); }
			void draw(Image img, const Space& s)
				{ img.blit(surface_, s); }
			void handle(const SDL_Event&);
		protected:
			virtual void onUpdate(uint) = 0;
			virtual void onRender( ) = 0;
			virtual void onEvent(const SDL_Event&) = 0;

		private:
			SDL_Window *window_;
			SDL_Surface *surface_;
			uint windowID_;
	
			bool hidden_, focus_, mouseOver_;
			Color default_;

			Timer time_;
			uint last_;
	};
}

#endif

