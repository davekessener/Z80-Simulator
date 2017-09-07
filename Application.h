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
#include "Command.h"

namespace z80
{
	class Application
	{
		typedef void (Application::*command_fn)(const Tokenizer&);
		typedef std::function<std::string(const uint8_t *)> deasm_fn;

		public:
			Application( );
			~Application( );
			void run( );
			void execute(const std::string&);
		private:
			void quit(const Tokenizer&);
			void load(const Tokenizer&);
			void reset(const Tokenizer&);
			void start(const Tokenizer&);
			void stop(const Tokenizer&);
			void step(const Tokenizer&);

		private:
			Z80 mCPU;
			Screen mScreen;
			Keyboard mKeyboard;
			StatusPort mStatus;
			ScreenWindow wScreen;
			RAMMonitor wRAM;
			winui::CommandWindow wTerminal;
			lib::Schedule mSchedule;
			std::map<std::string, command_fn> mInstructions;
			deasm_fn fnDeassemble;

			bool cpu_running;
	};
}

#endif

