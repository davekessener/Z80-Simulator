#ifndef LIB_WINUI_IMAGE_H
#define LIB_WINUI_IMAGE_H

#include <string>

#include <SDL.h>

#include "WinUI.h"

namespace winui
{
	class Image
	{
		public:
			Image(const Image&);
			Image& operator=(const Image&);

		public:
			Image(const std::string&);
			~Image( );
			Image region(const Space&) const;
			uint getWidth( ) const { return region_.dim.w; }
			uint getHeight( ) const { return region_.dim.h; }
			void blit(SDL_Surface *, const Space&);
			SDL_Surface *getSurface( ) { return surface_; }
		private:
			void free( );

		private:
			SDL_Surface *surface_;
			uint *c_;
			Space region_;
	};
}

#endif

