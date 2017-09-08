#ifndef LIB_SDL_WINDOW_H
#define LIB_SDL_WINDOW_H

#include <functional>
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
		typedef std::function<void(void)> render_fn;
		typedef std::function<void(uint)> update_fn;
		typedef std::function<void(const SDL_Event&)> event_fn;

		public:
			Window(const std::string&, const Space&);
			~Window( );
			void hide( );
			void show( );
			bool isHidden( ) const { return hidden_; }
			bool hasFocus( ) const { return focus_; }
			void update( );
			void render( );
			void clear( );
			void setDefaultColor(const Color& c) { default_ = c; }
			void onRender(render_fn f) { render_ = f; }
			void onUpdate(update_fn f) { update_ = f; }
			void onEvent(event_fn f) { event_ = f; }
			void setWindowIcon(Image);
			void draw(Image img, const Position& p)
				{ draw(img, Space(p.x, p.y, img.getWidth(), img.getHeight())); }
			void draw(Image img, const Space& s)
				{ img.blit(surface_, s); }
			void handle(const SDL_Event&);
		private:
			SDL_Window *window_;
			SDL_Surface *surface_;
			int windowID_;
	
			bool hidden_, focus_;
			Color default_;
			render_fn render_;
			update_fn update_;
			event_fn event_;

			Timer time_;
			uint last_;
	};
}

#endif

