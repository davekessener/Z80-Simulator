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

namespace z80 {

using winui::Manager;
using winui::Dimension;
using winui::Image;
using winui::Color;

Application::Application(void)
	: wScreen(mScreen, mKeyboard)
	, wTerminal(MXT_TERMINAL_TITLE, Dimension(MXT_COLS, MXT_ROWS), Image(MXT_CHARSET_PATH), Dimension(MXT_CHAR_W, MXT_CHAR_H), MXT_CHARSET_COLORSPACE)
{
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

	mInstructions[CMD_QUIT] = &Application::quit;
	mInstructions[CMD_LOAD] = &Application::load;
	mInstructions[CMD_RESET] = &Application::reset;
	mInstructions[CMD_START] = &Application::start;
	mInstructions[CMD_STOP] = &Application::stop;
	mInstructions[CMD_RUN] = &Application::start;
	mInstructions[CMD_STEP] = &Application::step;

	fnDeassemble = [](const uint8_t *p) { return lib::stringf("0x%02X", *p); };

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
	wTerminal.println(lib::stringf("Executing @$%04X: %s", mCPU.getPC(), fnDeassemble(&mCPU.RAM(mCPU.getPC())).c_str()));
	mCPU.execute();
}

}

