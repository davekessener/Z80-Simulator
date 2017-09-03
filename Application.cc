#include "Application.h"
#include "Image.h"

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

namespace z80 {

using winui::Manager;
using winui::Dimension;
using winui::Image;
using winui::Color;

Application::Application(void)
	: wScreen_(screen_, keyboard_)
	, wTerminal_(MXT_TERMINAL_TITLE, Dimension(MXT_COLS, MXT_ROWS), Image(MXT_CHARSET_PATH), Dimension(MXT_CHAR_W, MXT_CHAR_H), MXT_CHARSET_COLORSPACE)
{
	cpu_.reset();

	cpu_.registerPeripheral(0x00, status_);
	cpu_.registerPeripheral(0x10, screen_);
	cpu_.registerPeripheral(0x20, keyboard_);

	status_.onInt([this]( ) { cpu_.interrupt(); });
	status_.registerInt(0x10, wScreen_.int60fps());
	status_.registerInt(0x20, keyboard_.keyPressedInt());

	wRAM_.setAccess([this](uint a) { return (a >= 0 && a < 0x10000) ? cpu_.getRAM(a) : -1; });

	schedule_.schedule([this]( ) { cpu_.execute(); }, 1);
	schedule_.schedule([]( ) { Manager::instance().tick(); }, 1);
	schedule_.schedule([]( ) { Manager::instance().render(); }, 1000000/60);

	wTerminal_.setPrompt(MXT_TERMINAL_PROMPT);
	wTerminal_.setBackgroundColor(Color::BLACK());
	wTerminal_.setFontColorIndex(MXT_COLOR_GREEN);
	wTerminal_.setExecutionHandler([this](const std::string& cmd) { execute(cmd); });
}

Application::~Application(void)
{
}

void Application::run(void)
{
	while(Manager::instance().isRunning())
	{
		schedule_.step();
	}
}

void Application::execute(const std::string& cmd)
{
	if(cmd == "quit")
	{
		Manager::instance().stop();
	}
}

}

