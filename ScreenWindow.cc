#include "ScreenWindow.h"

#define MXT_60FPS (1000/60)

namespace z80 {

ScreenWindow::ScreenWindow(Screen& screen, Keyboard& keyboard)
	: window_(MXT_SCREEN_TITLE, winui::Space(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, MXT_SCREEN_WIDTH, MXT_SCREEN_HEIGHT))
	, charset_(MXT_CHARSET)
	, screen_(&screen)
	, keyboard_(&keyboard)
	, blink_(0)
	, fps_(0)
	, int_(new bool)
{
	window_.onUpdate([this](uint ms) { onUpdate(ms); });
	window_.onRender([this]( ) { onRender(); });
	window_.onEvent([this](const SDL_Event& e) { onEvent(e); });

	int_.set(false);
}

void ScreenWindow::onUpdate(uint ms)
{
	blink_ = (blink_ + ms) % MXT_BLINK;
	
	if((fps_ += ms) >= MXT_60FPS)
	{
		fps_ = fps_ % MXT_60FPS;
		if(screen_->timer_en()) int_.set(true);
	}
}

void ScreenWindow::onRender(void)
{
	const uint cx = screen_->getCursorX();
	const uint cy = screen_->getCursorY();

	for(uint y = 0 ; y < MXT_SCREEN_ROWS ; ++y)
	{
		for(uint x = 0 ; x < MXT_SCREEN_COLS ; ++x)
		{
			uint8_t c = screen_->getChar(x, y);
			bool blink = isBlinking() && screen_->cursor_en() && cx == x && cy == y;

			blink = (blink ? 1 : 0) ^ ((c & 0x80) ? 1 : 0);

			drawChar(x, y, (char)(c & 0x7F), blink);
		}
	}
}

void ScreenWindow::onEvent(const SDL_Event& e)
{

	if(window_.hasFocus()) switch(e.type)
	{
		case SDL_KEYDOWN:
			keyboard_->press(e.key.keysym.sym, true);
			break;
		case SDL_KEYUP:
			keyboard_->press(e.key.keysym.sym, false);
			break;
	}
}

void ScreenWindow::drawChar(uint x, uint y, char c, bool blink)
{
	winui::Space s(c * MXT_CHARW, blink ? MXT_CHARH : 0, MXT_CHARW, MXT_CHARH);

	window_.draw(charset_.region(s), winui::Position(x * MXT_CHARW, y * MXT_CHARH));
}

}

