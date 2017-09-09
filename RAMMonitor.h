#ifndef Z80_RAMMONITOR_H
#define Z80_RAMMONITOR_H

#include <map>
#include <deque>
#include <stack>

#include "Window.h"
#include "Image.h"

#define MXT_VIEWSIZE 0x200

namespace z80
{
	class RAMMonitor
	{
		enum class State
		{
			DEFAULT,
			EDITING,
			GOTO
		};

		struct Change
		{
			uint16_t addr;
			uint8_t prev, next;
		};

		public:
		typedef std::function<uint8_t&(uint16_t)> access_fn;
		typedef std::function<std::string(void)> render_fn;
		typedef std::stack<Change> stack_t;

		public:
			RAMMonitor(uint = MXT_VIEWSIZE);
			void setWatchAddress(uint a) { vOff_ = a & ~0x0F; addr_ = a; }
			void setAccess(access_fn f) { access_ = f; }
			void setWindowIcon(winui::Image i) { window_.setWindowIcon(i); }
			void show( ) { window_.show(); }
			void hide( ) { window_.hide(); }
		private:
			void onUpdate(uint);
			void onRender( );
			void onEventDefault(const SDL_Event&);
			void onEventGoto(const SDL_Event&);
			void onEventEditing(const SDL_Event&);
			std::string renderDefault( );
			std::string renderGoto( );
			std::string renderEditing( );
			void click(int, int);
			void scroll(int);
			void push(stack_t&, uint16_t, uint8_t, uint8_t);
			void act(stack_t&, stack_t&);
			void undo( ) { act(undo_, redo_); }
			void redo( ) { act(redo_, undo_); }
			void gotoAddr(uint16_t);
			void setGotoCursor( );

		private:
			winui::Window window_;
			winui::Image charset_;
			uint16_t viewsize_, addr_, vOff_;
			access_fn access_;
			render_fn renderFooder_;
			std::string msg_;
			State state_;
			uint8_t buf_;
			std::map<uint, uint8_t> key_;
			stack_t undo_, redo_;
			uint8_t aBuf_[4];
			int aBufPos_;
			int cx_, cy_;
			uint blink_;
	};
}

#endif

