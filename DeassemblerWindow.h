#ifndef Z80_DEASSEMBLERWINDOW_H
#define Z80_DEASSEMBLERWINDOW_H

#include <functional>
#include <string>
#include <map>

#include "lib.h"
#include "Z80.h"
#include "CharacterWindow.h"
#include "Image.h"
#include "Disassemble.h"

#define MXT_LINECOUNT 59

namespace z80
{
	class DeassemblerWindow : public winui::CharacterWindow
	{
		typedef std::map<uint16_t, std::string> map_t;
		typedef std::function<bool(uint16_t)> check_break_fn;
		typedef std::function<void(uint16_t)> set_break_fn;
		typedef std::pair<check_break_fn, set_break_fn> break_t;

		static const uint COLOR_BLACK   = 0x00; // ---
		static const uint COLOR_BLUE    = 0x01; // --B
		static const uint COLOR_GREEN   = 0x02; // -G-
		static const uint COLOR_CYAN    = 0x03; // -GB
		static const uint COLOR_RED     = 0x04; // R--
		static const uint COLOR_MAGENTA = 0x05; // R-B
		static const uint COLOR_YELLOW  = 0x06; // RG-
		static const uint COLOR_WHITE   = 0x07; // RGB

		enum class State
		{
			DEFAULT,
			GOTO
		};

		public:
			DeassemblerWindow(Z80&, uint = MXT_LINECOUNT);
			virtual ~DeassemblerWindow( );
			void setAddress(uint16_t a) { addr_ = a; }
			void setLabelMap(const map_t& m) { map_ = m; }
			void setFollowPC(bool v) { followPC_ = v; }
			void setBreakPointCallback(break_t p) { checkBreak_ = p.first; setBreak_ = p.second; }
		private:
			void onUpdate(uint);
			void onRender( );
			void onEvent(const SDL_Event&);
			void onEventDefault(const SDL_Event&);
			void onEventGoto(const SDL_Event&);
			std::string getFooder( );
			void renderLine(uint, uint16_t, const Instruction&);
			void renderEmptyLine(uint);
			void scroll(int);
			void click(int, bool);
			void clearBuf( ) { aBuf_[0] = aBuf_[1] = aBuf_[2] = aBuf_[3] = aBufPos_ = 0; }
			void setGotoCursor( );
			void gotoAddr(uint16_t);

		private:
			map_t map_;
			uint cLines_, off_;
			uint16_t pc_, addr_, sel_;
			Z80 *cpu_;
			bool followPC_, scrollable_;
			check_break_fn checkBreak_;
			set_break_fn setBreak_;
			std::map<uint, uint16_t> addresses_;
			uint8_t aBuf_[4];
			int aBufPos_;
			State state_;

			std::map<uint, uint8_t> key_;
	};
}

#endif

