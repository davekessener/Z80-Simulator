#ifndef Z80_RAMMONITOR_H
#define Z80_RAMMONITOR_H

#include "Window.h"
#include "Image.h"

#define MXT_VIEWSIZE 0x200

namespace z80
{
	class RAMMonitor
	{
		public:
		typedef std::function<int(uint)> access_fn;
		typedef std::function<std::string(void)> render_fn;

		public:
			RAMMonitor(uint = MXT_VIEWSIZE);
			void setWatchAddress(uint a) { vOff_ = a & ~0x0F; addr_ = a; }
			void setAccess(access_fn f) { access_ = f; }
		private:
			void onUpdate(uint);
			void onRender( );
			void onEventDefault(const SDL_Event&);
			void onEventGoto(const SDL_Event&);
			std::string renderDefault( );

		private:
			winui::Window window_;
			winui::Image charset_;
			uint viewsize_, addr_, vOff_;
			access_fn access_;
			render_fn renderFooder_;
			std::string msg_;
	};
}

#endif

