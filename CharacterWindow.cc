#include "CharacterWindow.h"

#define MXT_BLINKSPEED 1000

namespace winui {

CharacterWindow::CharacterWindow(const std::string& title, Position p, Dimension size, Image charset, Dimension charsize, uint colorspace)
	: Window(title, Space(p.x, p.y, size.w * charsize.w, size.h * charsize.h))
	, charset_(charset)
{
	colorspace_ = colorspace;
	winsize_ = size;
	charsize_ = charsize;
	blink_ = 0;
	blinkspeed_ = MXT_BLINKSPEED;
	color_ = 0;
	en_blink_ = blinkIndependently_ = false;
}

void CharacterWindow::onUpdate(uint ms)
{
	blink_ = (blink_ + ms) % blinkspeed_;
}

void CharacterWindow::renderChar(Position p, uint8_t ch, uint c, bool inv)
{
	if(p.x < 0 || p.y < 0 || (uint)p.x >= winsize_.w || (uint)p.y >= winsize_.h)
		return;
	
	if(isBlinking() && p.x == cursor_.x && p.y == cursor_.y)
	{
		inv = !inv;
	}

	Space s(ch * charsize_.w, (c + (inv ? colorspace_ : 0)) * charsize_.h, charsize_.w, charsize_.h);

	draw(charset_.region(s), Position(p.x * charsize_.w, p.y * charsize_.h));
}

void CharacterWindow::renderString(Position p, const std::string& s)
{
	auto i1 = s.cbegin(), i2 = s.cend();
	int x = p.x;

	for(int y = p.y ; y < (int)winsize_.h ; ++y)
	{
		for( ; x < (int)winsize_.w ; ++x)
		{
			if(i1 == i2) return;

			renderChar(Position(x, y), *i1++, color_, false);
		}

		x = 0;
	}
}

}

