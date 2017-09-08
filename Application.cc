#include "Application.h"
#include "Image.h"
#include "lib.h"

#define MXT_TERMINAL_TITLE "Z80 Simulation Terminal"
#define MXT_TERMINAL_PROMPT "$> "
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

#define MXT_ICON_PATH "z80.bmp"

namespace z80 {

using winui::Manager;
using winui::Dimension;
using winui::Image;
using winui::Color;

Application::Application(void)
	: wScreen(mScreen, mKeyboard)
	, wStatus(mCPU)
	, wTerminal(MXT_TERMINAL_TITLE, Dimension(MXT_COLS, MXT_ROWS), Image(MXT_CHARSET_PATH), Dimension(MXT_CHAR_W, MXT_CHAR_H), MXT_CHARSET_COLORSPACE)
{
	Image icon(MXT_ICON_PATH);

	wScreen.setWindowIcon(icon);
	wStatus.setWindowIcon(icon);
	wRAM.setWindowIcon(icon);
	wTerminal.setWindowIcon(icon);

	mCPU.reset();

	mCPU.registerPeripheral(0x00, mStatus);
	mCPU.registerPeripheral(0x10, mScreen);
	mCPU.registerPeripheral(0x20, mKeyboard);

	mStatus.onInt([this]( ) { mCPU.interrupt(); });
	mStatus.registerInt(0x10, wScreen.int60fps());
	mStatus.registerInt(0x20, mKeyboard.keyPressedInt());

	wRAM.setAccess([this](uint a) { return (a >= 0 && a < 0x10000) ? mCPU.RAM(a) : -1; });

	mSchedule.schedule([this]( ) { if(cpu_running) mCPU.execute(); }, 1);
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
	mCPU.reset();
	cpu_running = false;
}

void Application::start(const Tokenizer& t)
{
	wTerminal.println(lib::stringf("Start running @$%04X", mCPU.getPC()));
	cpu_running = true;
}

void Application::stop(const Tokenizer& t)
{
	cpu_running = true;
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
	if(t.size() != 3 || t[1].type != TokenType::LITERAL || t[2].type != TokenType::NUMBER)
	{
		throw std::string("SET R $V");
	}

	std::string r(t[1].token);
	uint16_t v(t[2].value & 0xFFFF);

	if(r == "pc")
	{
		mCPU.setPC(v);
		wTerminal.println(lib::stringf("Set PC to $%04X", v));
	}
	else if(r == "sp")
	{
		mCPU.setSP(v);
		wTerminal.println(lib::stringf("Set SP to $%04X", v));
	}
	else if(r == "af")
	{
		mCPU.setAF(v);
		wTerminal.println(lib::stringf("Set AF to $%04X", v));
	}
	else if(r == "bc")
	{
		mCPU.setBC(v);
		wTerminal.println(lib::stringf("Set BC to $%04X", v));
	}
	else if(r == "de")
	{
		mCPU.setDE(v);
		wTerminal.println(lib::stringf("Set DE to $%04X", v));
	}
	else if(r == "hl")
	{
		mCPU.setHL(v);
		wTerminal.println(lib::stringf("Set HL to $%04X", v));
	}
	else if(r == "ix")
	{
		mCPU.setIX(v);
		wTerminal.println(lib::stringf("Set IX to $%04X", v));
	}
	else if(r == "iy")
	{
		mCPU.setIY(v);
		wTerminal.println(lib::stringf("Set IY to $%04X", v));
	}
	else
	{
		throw lib::stringf("Unknown register '%s'!", r.c_str());
	}
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

}

