#ifndef LIB_WINUI_MANAGER_H
#define LIB_WINUI_MANAGER_H

#include <string>
#include <map>

#include <SDL.h>

#include "WinUI.h"
#include "Window.h"
#include "Image.h"

namespace winui
{
	class Manager
	{
		public:
			static Manager& instance( );

			bool isRunning( ) const { return running_; }
			void tick( );
			void render( );
			void registerWindow(int, Window&);
			void unregisterWindow(int);
			SDL_Surface *loadImage(const std::string&);
			void unloadImage(SDL_Surface *);
			void stop( ) { running_ = false; }

		private:
			std::map<int, Window*> windows_;
			std::map<std::string, SDL_Surface*> images_;
			std::map<SDL_Surface*, std::string> r_images_;
			std::map<SDL_Surface*, int> imgCounts_;
			bool running_;

		private:
			Manager( );
			~Manager( );
			Manager(const Manager&) = delete;
			Manager& operator=(const Manager&) = delete;
	};
}

#endif

