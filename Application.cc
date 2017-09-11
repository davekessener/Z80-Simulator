#include <algorithm>

#include "Application.h"
#include "Image.h"
#include "lib.h"

#define MXT_TERMINAL_TITLE "Z80 Terminal"
#define MXT_TERMINAL_PROMPT "> "
#define MXT_COLS 80
#define MXT_ROWS 20
#define MXT_CHARSET_PATH "charset.bmp"
#define MXT_CHARSET_COLORSPACE 8
#define MXT_CHAR_W 8
#define MXT_CHAR_H 12
#define MXT_COLOR_BLACK 0
#define MXT_COLOR_BLUE 1
#define MXT_COLOR_GREEN 2
#define MXT_COLOR_CYAN 3
#define MXT_COLOR_RED 4
#define MXT_COLOR_MAGENTA 5
#define MXT_COLOR_YELLOW 6
#define MXT_COLOR_WHITE 7

#define CMD_QUIT "quit"
#define CMD_LOAD "load"
#define CMD_RESET "reset"
#define CMD_START "start"
#define CMD_STOP "stop"
#define CMD_RUN "run"
#define CMD_STEP "step"
#define CMD_HELP "help"
#define CMD_SET "set"
#define CMD_SHOW "show"
#define CMD_HIDE "hide"
#define CMD_INT "int"
#define CMD_BREAK "break"

#define MXT_ICON_PATH "z80.bmp"

namespace z80 {

using winui::Manager;
using winui::Dimension;
using winui::Image;
using winui::Color;

Application::Application(void)
	: wScreen(mScreen, mKeyboard)
	, wStatus(mCPU)
	, wDeASM(mCPU)
	, wTerminal(MXT_TERMINAL_TITLE, Dimension(MXT_COLS, MXT_ROWS), Image(MXT_CHARSET_PATH), Dimension(MXT_CHAR_W, MXT_CHAR_H), MXT_CHARSET_COLORSPACE)
{
	reset();

	mCPU.registerPeripheral(0x00, mStatus);
	mCPU.registerPeripheral(0x10, mScreen);
	mCPU.registerPeripheral(0x20, mKeyboard);

	mStatus.onInt([this]( ) { mCPU.interrupt(); });
	mStatus.registerInt(0x01, wScreen.int60fps());
	mStatus.registerInt(0x02, mKeyboard.keyPressedInt());
	mStatus.registerInt(0xFF, manualInt);

	wRAM.setAccess([this](uint16_t a) -> uint8_t& { return mCPU.RAM(a); });

	wDeASM.setBreakPointCallback(std::make_pair(
		[this](uint16_t a) -> bool { return std::find(breakPoints.begin(), breakPoints.end(), a) != breakPoints.end(); },
		[this](uint16_t a) -> void { toggleBreakpoint(a); }));

	mSchedule.schedule([this]( ) { tick(); }, 1);
	mSchedule.schedule([]( ) { Manager::instance().tick(); }, 1);
	mSchedule.schedule([]( ) { Manager::instance().render(); }, 1000000/60);

	wTerminal.setPrompt(MXT_TERMINAL_PROMPT);
	wTerminal.setBackgroundColor(Color::BLACK());
	wTerminal.setFontColorIndex(MXT_COLOR_GREEN);
	wTerminal.setExecutionHandler([this](const std::string& cmd) { execute(cmd); });
	wTerminal.setCloseHandler([]( ) { Manager::instance().stop(); });

	mInstructions[CMD_QUIT]  = &Application::quit;
	mInstructions[CMD_LOAD]  = &Application::load;
	mInstructions[CMD_RESET] = &Application::reset;
	mInstructions[CMD_START] = &Application::start;
	mInstructions[CMD_STOP]  = &Application::stop;
	mInstructions[CMD_RUN]   = &Application::start;
	mInstructions[CMD_STEP]  = &Application::step;
	mInstructions[CMD_HELP]  = &Application::help;
	mInstructions[CMD_SET]   = &Application::set;
	mInstructions[CMD_SHOW]  = &Application::show;
	mInstructions[CMD_HIDE]  = &Application::hide;
	mInstructions[CMD_INT]   = &Application::interrupt;
	mInstructions[CMD_BREAK] = &Application::setBreak;

#define MAKE_SET(R) \
std::make_pair( \
TokenType::NUMBER, \
[this](const Tokenizer::Token& t) { \
mCPU.set##R (t.value); \
wTerminal.println(lib::stringf("Setting " #R " to $%04X", t.value)); \
})
	mSetFunctions["pc"] = MAKE_SET(PC);
	mSetFunctions["sp"] = MAKE_SET(SP);
	mSetFunctions["af"] = MAKE_SET(AF);
	mSetFunctions["bc"] = MAKE_SET(BC);
	mSetFunctions["de"] = MAKE_SET(DE);
	mSetFunctions["hl"] = MAKE_SET(HL);
	mSetFunctions["ix"] = MAKE_SET(IX);
	mSetFunctions["iy"] = MAKE_SET(IY);
#undef MAKE_SET

	cpu_running = false;
}

Application::~Application(void)
{
}

void Application::run(void)
{
	while(Manager::instance().isRunning())
	{
		mSchedule.step();
	}
}

void Application::execute(const std::string& cmd)
{
	try
	{
		Tokenizer t(cmd);

		if(t.begin() == t.end())
			return;

		const auto& ins(*t.begin());

		if(ins.type != TokenType::LITERAL)
		{
			throw std::string("Invalid command!");
		}
		else if(mInstructions.count(ins.token))
		{
			(this->*mInstructions[ins.token])(t);
		}
		else
		{
			throw std::string("Unknown command [") + ins.token + "] !";
		}
	}
	catch(const std::string& e)
	{
		wTerminal.println("ERR: " + e);
	}
}

void Application::tick(void)
{
	if(cpu_running)
	{
		uint32_t skip = skipBP_;

		skipBP_ = 0x10000;

		for(const auto& p : breakPoints)
		{
			if(mCPU.getPC() != skip && mCPU.getPC() == p)
			{
				cpu_running = false;
				wTerminal.println(lib::stringf("BREAK @$%04X: %s", p, mCPU.disassemble(p).c_str()));
				return;
			}
		}

		mCPU.execute();
	}
}

void Application::reset(void)
{
	mCPU.reset();
	mScreen.reset();
	mKeyboard.reset();
	mStatus.reset();
	cpu_running = false;
}

void Application::toggleBreakpoint(uint16_t p)
{
	auto i = std::find(breakPoints.begin(), breakPoints.end(), p);

	if(i == breakPoints.end())
	{
		breakPoints.push_back(p);
		wTerminal.println(lib::stringf("Added breakpoint @$%04X", p));
	}
	else
	{
		breakPoints.erase(i);
		wTerminal.println(lib::stringf("Removed breakpoint @$%04X", p));
	}
}

// # --------------------------------------------------------------------------- 

void Application::quit(const Tokenizer& t)
{
	wTerminal.println("Shutting down ...");
	Manager::instance().stop();
}

void Application::load(const Tokenizer& t)
{
	if(t.size() < 2 || t[1].type != TokenType::STRING)
	{
		throw std::string("LOAD \"FILE.EXT\" [OFFSET]");
	}

	uint16_t addr = 0;

	if(t.size() >= 3 && t[2].type == TokenType::NUMBER)
	{
		addr = t[2].value & 0xFFFF;
	}

	std::string fn(t[1].token);
	Program prg(fn);

	wTerminal.println(lib::stringf("Loading \"%s\" [%uB] @$%04X ...", fn.c_str(), prg.length(), addr));

	mCPU.loadRAM(addr, prg);
}

void Application::reset(const Tokenizer& t)
{
	wTerminal.println("Resetting CPU.");
	reset();
}

void Application::start(const Tokenizer& t)
{
	wTerminal.println(lib::stringf("Start running @$%04X", mCPU.getPC()));
	cpu_running = true;
	skipBP_ = mCPU.getPC();
}

void Application::stop(const Tokenizer& t)
{
	cpu_running = false;
}

void Application::step(const Tokenizer& t)
{
	wTerminal.println(lib::stringf("Executing @$%04X: %s", mCPU.getPC(), mCPU.disassemble(mCPU.getPC()).c_str()));
	mCPU.execute();
}

void Application::help(const Tokenizer& t)
{
	if(t.size() < 2)
	{
		for(const auto& p : mInstructions)
		{
			wTerminal.println(p.first);
		}
	}
}

void Application::set(const Tokenizer& t)
{
	if(t.size() != 3 || t[1].type != TokenType::LITERAL)
	{
		throw std::string("SET ID VALUE");
	}

	std::string r(t[1].token);
	Tokenizer::Token tk(t[2]);

	auto i = mSetFunctions.find(r);

	if(i == mSetFunctions.end())
	{
		throw std::string("Unknown ID '" + r + "'!");
	}
	else if(i->second.first != tk.type)
	{
		throw std::string("Invalid value type; expected " + toString(i->second.first) + ", not " + toString(tk.type) + "!");
	}

	i->second.second(tk);
}

void Application::show(const Tokenizer& t)
{
	if(t.size() != 2 || t[1].type != TokenType::LITERAL)
	{
		throw std::string("SHOW <SCREEN|RAM|STATUS>");
	}

	std::string s(t[1].token);

	if(s == "screen")
	{
		wScreen.show();
	}
	else if(s == "ram")
	{
		wRAM.show();
	}
	else if(s == "status")
	{
		wStatus.show();
	}
	else
	{
		throw std::string("Unknown window '") + s + "'!";
	}
}

void Application::hide(const Tokenizer& t)
{
	if(t.size() != 2 || t[1].type != TokenType::LITERAL)
	{
		throw std::string("HIDE <SCREEN|RAM|STATUS>");
	}

	std::string s(t[1].token);

	if(s == "screen")
	{
		wScreen.hide();
	}
	else if(s == "ram")
	{
		wRAM.hide();
	}
	else if(s == "status")
	{
		wStatus.hide();
	}
	else
	{
		throw std::string("Unknown window '") + s + "'!";
	}
}

void Application::interrupt(const Tokenizer& t)
{
	manualInt.set(true);

	if(manualInt.get())
	{
		throw std::string("Failed to reset interrupt line.");
	}
}

void Application::setBreak(const Tokenizer& t)
{
	if(t.size() != 2)
	{
		throw std::string("BREAK CLEAR|LIST|$ADDR");
	}

	if(t[1].type == TokenType::NUMBER)
	{
		toggleBreakpoint(t[1].value);
	}
	else if(t[1].type == TokenType::LITERAL)
	{
		if(t[1].token == "clear")
		{
			wTerminal.println(lib::stringf("Removed all %u breakpoints.", breakPoints.size()));
			breakPoints.clear();
		}
		else if(t[1].token == "list")
		{
			wTerminal.println("Breakpoints:");
			for(const auto& p : breakPoints)
			{
				wTerminal.println(lib::stringf("@$%04X", p));
			}
		}
		else
		{
			throw std::string("Invalid argument '" + t[1].token + "' to command 'BREAK'!");
		}
	}
}

}

