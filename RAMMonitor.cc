#include <stdlib.h>
#include <stdio.h>

#include "RAMMonitor.h"

#define MXT_BUFSIZE 100

#define CHAR_W 8
#define CHAR_H 12
#define CHAR_COLORSPACE 8
#define CHARSET_PATH "charset.bmp"

#define WIN_TITLE "Z80 RAM Monitor"
#define WIN_WIDTH (4+3+3*16)
#define WIN_HEIGHT (2+2)

namespace z80 {

using winui::Position;
using winui::Dimension;
using winui::Space;
using winui::Color;

RAMMonitor::RAMMonitor(uint viewsize)
	: window_(WIN_TITLE, Space(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_WIDTH*CHAR_W, (WIN_HEIGHT+(viewsize+15)/16)*CHAR_H))
	, charset_(CHARSET_PATH)
	, viewsize_(viewsize)
	, addr_(0)
	, vOff_(0)
{
	window_.setDefaultColor(Color::WHITE());

	window_.onUpdate([this](uint ms) { onUpdate(ms); });
	window_.onRender([this]( ) { onRender(); });
	window_.onEvent([this](const SDL_Event& e) { onEventDefault(e); });

	access_ = [](uint a) { return -1; };
	renderFooder_ = [this]( ) { return renderDefault(); };
}

void RAMMonitor::onUpdate(uint ms)
{
}

void RAMMonitor::onRender(void)
{
	char buf[MXT_BUFSIZE];

	window_.clear();

	uint x = 0, y = 0;
	const uint cx = 4 + 3 + (addr_ % 16) * 3, cy = 2 + addr_ / 16 - vOff_ / 16;
	uint color = 0;

	auto nl = [this, &x, &y]( )
	{
		x = 0;
		++y;
	};

	auto outCh = [this, &x, &y, &nl, &color, cx, cy](uint8_t ch, bool highlight, bool inv)
	{
		uint c = color;

		if(highlight)
		{
			if((x == cx || x == cx + 1) && y == cy)
			{
				c = 4; // RED
				inv = true;
			}
			else if(x == cx || x == cx + 1 || y == cy)
			{
				c = 3; // CYAN
				inv = true;
			}
		}

		Space s(ch * CHAR_W, (c + (inv ? CHAR_COLORSPACE : 0)) * CHAR_H, CHAR_W, CHAR_H);

		window_.draw(charset_.region(s), Position(x * CHAR_W, y * CHAR_H));

		if(++x == WIN_WIDTH) nl();
	};

	auto outStr = [this, &outCh](const std::string& s, bool hl = true)
	{
		for(const auto& e : s) outCh(e, hl, false);
	};

#define X_S_V 129
#define X_H_H 139
#define X_HH_SV 159
#define X_HH_TSV 154

	outStr("     ");
	outCh(X_S_V, true, false);
	outStr(" 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F ");
	for(int i = 0 ; i < WIN_WIDTH ; ++i) outCh(i == 5 ? X_HH_SV : X_H_H, true, false);
	
	for(uint a1 = vOff_ / 16, a2 = a1 + (viewsize_ + 15) / 16 ; a1 < a2 ; ++a1)
	{
		snprintf(buf, MXT_BUFSIZE, "%04X %c ", a1 * 16, X_S_V);
		outStr(buf);

		for(uint i = 0 ; i < 16 ; ++i)
		{
			int v = access_(i + a1 * 16);

			if(v >= 0 && v < 0x100)
			{
				snprintf(buf, MXT_BUFSIZE, "%02X ", v);
				outStr(buf);
			}
			else
			{
				outStr("   ");
			}
		}
	}

	for(uint i = 0 ; i < WIN_WIDTH ; ++i) outCh(i == 5 ? X_HH_TSV : X_H_H, true, false);
	
	if(msg_.size() > WIN_WIDTH) msg_ = "ERR: Msg too large!";

	x = WIN_WIDTH - msg_.size();
	color = 4;
	outStr(msg_, false);

	x = 0;
	color = 0;

	outStr(renderFooder_().c_str(), false);
}

void RAMMonitor::onEventDefault(const SDL_Event& e)
{
	if(window_.hasFocus())
	{
		switch(e.type)
		{
			case SDL_KEYDOWN:
				switch(e.key.keysym.sym)
				{
					case SDLK_LEFT:
						if(addr_ & 0x0F) --addr_;
						break;
					case SDLK_RIGHT:
						if((addr_ & 0x0F) < 0x0F) ++addr_;
						break;
					case SDLK_UP:
						if(addr_ >= 0x10)
						{
							if((addr_ -= 0x10) < vOff_)
							{
								vOff_ -= 0x10;
							}
						}
						break;
					case SDLK_DOWN:
						if((addr_ + 0x10) > addr_)
						{
							if((addr_ += 0x10) >= vOff_ + viewsize_)
							{
								vOff_ += 0x10;
							}
						}
						break;
				}
				break;
		}
	}
}

std::string RAMMonitor::renderDefault(void)
{
	char buf[MXT_BUFSIZE];
	snprintf(buf, MXT_BUFSIZE, "$%04X: 0x%02X", addr_, access_(addr_));
	return std::string(buf);
}

}

