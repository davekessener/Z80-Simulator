#include "ScreenWindow.h"

#define MXT_SCREEN_TITLE "Z80 Screen Simulator"
#define MXT_CHARW 8
#define MXT_CHARH 8
#define MXT_SCREEN_COLS 80
#define MXT_SCREEN_ROWS 60
#define MXT_SCREEN_WIDTH (MXT_SCREEN_COLS*MXT_CHARW)
#define MXT_SCREEN_HEIGHT (MXT_SCREEN_ROWS*MXT_CHARH)
#define MXT_CHARSET "dascii.bmp"

#define CHAR_COLORSPACE 1

#define MXT_BLINK 1000

#define MXT_60FPS (1000/60)

namespace z80 {

using winui::Position;
using winui::Dimension;
using winui::Space;
using winui::Image;
using winui::CharacterWindow;

ScreenWindow::ScreenWindow(Screen& screen, Keyboard& keyboard)
	: CharacterWindow(MXT_SCREEN_TITLE, Position::CENTER(), Dimension(MXT_SCREEN_COLS, MXT_SCREEN_ROWS), Image(MXT_CHARSET), Dimension(MXT_CHARW, MXT_CHARH), CHAR_COLORSPACE)
	, screen_(&screen)
	, keyboard_(&keyboard)
	, fps_(0)
	, int_(new bool)
{
	int_.set(false);
	blinkIndependently(true);
}

void ScreenWindow::onUpdate(uint ms)
{
	CharacterWindow::onUpdate(ms);
	
	if((fps_ += ms) >= MXT_60FPS)
	{
		fps_ = fps_ % MXT_60FPS;
		if(screen_->timer_en()) int_.set(true);
	}
}

void ScreenWindow::onRender(void)
{
	enableBlink(screen_->cursor_en());
	updateCursor(Position(screen_->getCursorX(), screen_->getCursorY()));

	for(uint y = 0 ; y < MXT_SCREEN_ROWS ; ++y)
	{
		for(uint x = 0 ; x < MXT_SCREEN_COLS ; ++x)
		{
			uint8_t c = screen_->getChar(x, y);

			renderChar(Position(x, y), (char)(c & 0x7F), 0, c & 0x80);
		}
	}
}

void ScreenWindow::onEvent(const SDL_Event& e)
{
	if(hasFocus()) switch(e.type)
	{
		case SDL_KEYDOWN:
			keyboard_->press(e.key.keysym.sym, true);
			break;
		case SDL_KEYUP:
			keyboard_->press(e.key.keysym.sym, false);
			break;
	}
}

}

