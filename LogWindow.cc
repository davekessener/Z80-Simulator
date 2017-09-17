#include "LogWindow.h"

#include "lib.h"

#define WIN_TITLE "Z80 Logger"
#define WIN_W 80
#define WIN_H 30
#define CHARSET_PATH "charset.bmp"
#define CHAR_W 8
#define CHAR_H 12
#define COLORSPACE 8

namespace z80 {

using winui::Dimension;
using winui::Position;
using winui::Space;
using winui::Image;
using winui::CharacterWindow;

LogWindow::LogWindow(void)
	: CharacterWindow(WIN_TITLE, Position::CENTER(), Dimension(WIN_W, WIN_H), Image(CHARSET_PATH), Dimension(CHAR_W, CHAR_H), COLORSPACE)
{
	off_ = 0;

	enableBlink(false);
	setLineWrap(false);
}

void LogWindow::println(const std::string& line)
{
	std::string s = line;
	Dimension size = getSize();

	while(!s.empty())
	{
		lines_.push_back(s.size() >= size.w ? s.substr(0, size.w) : s);
		s = s.size() >= size.w ? s.substr(size.w) : "";
		if(off_ > 0) ++off_;
	}
}

void LogWindow::onRender(void)
{
	Dimension size = getSize();
	uint start = 0;

	clear();

	if(lines_.size() >= size.h)
	{
		start = lines_.size() - size.h;
		if(start >= off_)
		{
			start -= off_;
		}
	}

	for(uint y = 0 ; y < size.h ; ++y)
	{
		if(start + y >= lines_.size()) break;
		renderString(Position(0, y), lib::stringf("%04u ", start + y) + lines_[start + y]);
	}
}

void LogWindow::onEvent(const SDL_Event& e)
{
	if(isMouseOver()) switch(e.type)
	{
		case SDL_MOUSEWHEEL:
			scroll((SDL_GetModState() & KMOD_SHIFT ? 10 : 1) * 
				e.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? -e.wheel.y : e.wheel.y);
			break;
	}
}

void LogWindow::scroll(int dy)
{
	Dimension size = getSize();

	if(lines_.size() > size.h)
	{
		int o = (int)off_ - dy;
		if(o < 0 ) o = 0;
		else if((uint)o > lines_.size() - size.h) o = lines_.size() - size.h;
		off_ = o;
	}
}

}

