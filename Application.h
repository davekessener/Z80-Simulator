#ifndef Z80_APPLICATION_H
#define Z80_APPLICATION_H

#include <vector>
#include <map>

#include "z80.h"
#include "Screen.h"
#include "StatusPort.h"
#include "ScreenWindow.h"
#include "RAMMonitor.h"
#include "StatusWindow.h"
#include "DeassemblerWindow.h"
#include "CommandWindow.h"
#include "Timer.h"
#include "Program.h"
#include "Manager.h"
#include "Schedule.h"
#include "Command.h"
#include "Property.h"

namespace z80
{
	class Application
	{
		typedef void (Application::*command_fn)(const Tokenizer&);
		typedef std::function<std::string(const uint8_t *)> deasm_fn;
		typedef std::function<void(const Tokenizer::Token&)> set_fn;
		typedef lib::Property<bool> int_t;

		public:
			Application( );
			~Application( );
			void run( );
			void execute(const std::string&);
		private:
			void tick( );
			void reset( );
			void toggleBreakpoint(uint16_t);

			void quit(const Tokenizer&);
			void load(const Tokenizer&);
			void reset(const Tokenizer&);
			void start(const Tokenizer&);
			void stop(const Tokenizer&);
			void step(const Tokenizer&);
			void help(const Tokenizer&);
			void set(const Tokenizer&);
			void show(const Tokenizer&);
			void hide(const Tokenizer&);
			void interrupt(const Tokenizer&);
			void setBreak(const Tokenizer&);

		private:
			Z80 mCPU;
			Screen mScreen;
			Keyboard mKeyboard;
			StatusPort mStatus;
			ScreenWindow wScreen;
			RAMMonitor wRAM;
			StatusWindow wStatus;
			DeassemblerWindow wDeASM;
			winui::CommandWindow wTerminal;
			lib::Schedule mSchedule;
			std::map<std::string, command_fn> mInstructions;
			std::map<std::string, std::pair<TokenType, set_fn>> mSetFunctions;

			bool cpu_running;
			int_t manualInt;
			std::vector<uint16_t> breakPoints;
			uint32_t skipBP_;
	};
}

#endif

