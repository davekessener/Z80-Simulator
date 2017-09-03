#ifndef Z80_APPLICATION_H
#define Z80_APPLICATION_H

#include "z80.h"
#include "Screen.h"
#include "StatusPort.h"
#include "ScreenWindow.h"
#include "RAMMonitor.h"
#include "CommandWindow.h"
#include "Timer.h"
#include "Program.h"
#include "Manager.h"
#include "Schedule.h"

namespace z80
{
	class Application
	{
		public:
			Application( );
			~Application( );
			void run( );
			void execute(const std::string&);
		private:
			Z80 cpu_;
			Screen screen_;
			Keyboard keyboard_;
			StatusPort status_;
			ScreenWindow wScreen_;
			RAMMonitor wRAM_;
			winui::CommandWindow wTerminal_;
			lib::Schedule schedule_;
	};
}

#endif

