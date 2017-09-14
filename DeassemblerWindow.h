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
			void renderLine(uint, uint16_t, const Instruction&);
			void scroll(int);

		private:
			map_t map_;
			uint cLines_;
			uint16_t pc_, addr_, sel_;
			Z80 *cpu_;
			bool followPC_;
			check_break_fn checkBreak_;
			set_break_fn setBreak_;
	};
}

#endif

