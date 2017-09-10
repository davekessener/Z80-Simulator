#ifndef Z80_DEASSEMBLERWINDOW_H
#define Z80_DEASSEMBLERWINDOW_H

#include <string>
#include <map>

#include "lib.h"
#include "Z80.h"
#include "Window.h"
#include "Image.h"
#include "Disassemble.h"

#define MXT_LINECOUNT 60

namespace z80
{
	class DeassemblerWindow
	{
		typedef std::map<uint16_t, std::string> map_t;

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
			~DeassemblerWindow( );
			void setAddress(uint16_t a) { addr_ = a; }
			void setLabelMap(const map_t& m) { map_ = m; }
			void setFollowPC(bool v) { followPC_ = v; }
		private:
			void onUpdate(uint);
			void onRender( );
			void onEvent(const SDL_Event&);
			void drawChar(uint, uint, uint8_t, uint = COLOR_BLACK, bool = false);
			void renderLine(uint, uint16_t, const Instruction&);

		private:
			winui::Window window_;
			winui::Image charset_;
			map_t map_;
			uint cLines_;
			uint16_t addr_;
			Z80 *cpu_;
			bool followPC_;
	};
}

#endif

