#include <stdlib.h>
#include <stdio.h>

#include "RAMMonitor.h"
#include "lib.h"

#define MXT_BLINK_F 1000

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
	window_.onEvent([this](const SDL_Event& e)
	{
		switch(state_)
		{
			case State::DEFAULT:
				onEventDefault(e);
				break;
			case State::EDITING:
				onEventEditing(e);
				break;
			case State::GOTO:
				onEventGoto(e);
				break;
		}
	});

	state_ = State::DEFAULT;

	renderFooder_ = [this]( )
	{
		switch(state_)
		{
			case State::DEFAULT:
				return renderDefault();
			case State::EDITING:
				return renderEditing();
			case State::GOTO:
				return renderGoto();
		}

		return std::string("");
	};

	key_[SDLK_0] = 0x00;
	key_[SDLK_1] = 0x01;
	key_[SDLK_2] = 0x02;
	key_[SDLK_3] = 0x03;
	key_[SDLK_4] = 0x04;
	key_[SDLK_5] = 0x05;
	key_[SDLK_6] = 0x06;
	key_[SDLK_7] = 0x07;
	key_[SDLK_8] = 0x08;
	key_[SDLK_9] = 0x09;
	key_[SDLK_a] = 0x0A;
	key_[SDLK_b] = 0x0B;
	key_[SDLK_c] = 0x0C;
	key_[SDLK_d] = 0x0D;
	key_[SDLK_e] = 0x0E;
	key_[SDLK_f] = 0x0F;

	cx_ = cy_ = -1;
	blink_ = 0;
}

void RAMMonitor::onUpdate(uint ms)
{
	blink_ = (blink_ + ms) % MXT_BLINK_F;
}

void RAMMonitor::onRender(void)
{
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

		if(cx_ >= 0 && cy_ >= 0 && (uint)cx_ == x && (uint)cy_ == y)
		{
			if(blink_ < MXT_BLINK_F / 2) inv = !inv;
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
		outStr(lib::stringf("%04X %c ", a1 * 16, X_S_V));

		for(uint i = 0 ; i < 16 ; ++i)
		{
			int v = access_(i + a1 * 16);

			if(v >= 0 && v < 0x100)
			{
				outStr(lib::stringf("%02X ", v));
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
						for(uint i = 0 ; i < (e.key.keysym.mod & KMOD_SHIFT ? 0x10 : 1) ; ++i)
						if(addr_ >= 0x10)
						{
							if((addr_ -= 0x10) < vOff_)
							{
								vOff_ -= 0x10;
							}
						}
						break;
					case SDLK_DOWN:
						for(uint i = 0 ; i < (e.key.keysym.mod & KMOD_SHIFT ? 0x10 : 1) ; ++i)
						if(addr_ + 0x10 < 0x10000)
						{
							if((addr_ += 0x10) >= vOff_ + viewsize_)
							{
								vOff_ += 0x10;
							}
						}
						break;
					case SDLK_g:
						state_ = State::GOTO;
						aBufPos_ = aBuf_[0] = aBuf_[1] = aBuf_[2] = aBuf_[3] = 0;
						setGotoCursor();
						break;
					case SDLK_u:
						undo();
						break;
					case SDLK_r:
						redo();
						break;
					case SDLK_0:
					case SDLK_1:
					case SDLK_2:
					case SDLK_3:
					case SDLK_4:
					case SDLK_5:
					case SDLK_6:
					case SDLK_7:
					case SDLK_8:
					case SDLK_9:
					case SDLK_a:
					case SDLK_b:
					case SDLK_c:
					case SDLK_d:
					case SDLK_e:
					case SDLK_f:
						state_ = State::EDITING;
						buf_ = access_(addr_);
						access_(addr_) = (buf_ & 0x0F) | (key_[e.key.keysym.sym] << 4);
						cx_ = 7 + (addr_ % 0x10) * 3 + 1;
						cy_ = 2 + (addr_ - vOff_) / 0x10;
						break;
				}
				break;
		}
	}

	if(window_.isMouseOver())
	{
		switch(e.type)
		{
			case SDL_MOUSEBUTTONDOWN:
				switch(e.button.button)
				{
					case SDL_BUTTON_LEFT:
						click(e.button.x, e.button.y);
						break;
				}
				break;
			case SDL_MOUSEWHEEL:
				scroll((SDL_GetModState() & KMOD_SHIFT ? 0x10 : 1) * 
						(e.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? -e.wheel.y : e.wheel.y));
				break;
		}
	}
}

void RAMMonitor::onEventGoto(const SDL_Event& e)
{
	if(window_.hasFocus()) switch(e.type)
	{
		case SDL_KEYDOWN:
			switch(e.key.keysym.sym)
			{
				case SDLK_ESCAPE:
					state_ = State::DEFAULT;
					break;
				case SDLK_RETURN:
					gotoAddr((aBuf_[0] << 12) | (aBuf_[1] << 8) | (aBuf_[2] << 4) | aBuf_[3]);
					state_ = State::DEFAULT;
					break;
				case SDLK_LEFT:
					if(aBufPos_ > 0) --aBufPos_;
					break;
				case SDLK_RIGHT:
					if(aBufPos_ < 3) ++aBufPos_;
					break;
				case SDLK_UP:
					aBufPos_ = 0;
					break;
				case SDLK_DOWN:
					aBufPos_ = 3;
					break;
				case SDLK_BACKSPACE:
					if(aBufPos_ > 0)
					{
						aBuf_[--aBufPos_] = 0;
					}
					break;
				case SDLK_DELETE:
					aBuf_[3] = aBuf_[2] = aBuf_[1] = aBuf_[0] = aBufPos_ = 0;
					break;
				case SDLK_0:
				case SDLK_1:
				case SDLK_2:
				case SDLK_3:
				case SDLK_4:
				case SDLK_5:
				case SDLK_6:
				case SDLK_7:
				case SDLK_8:
				case SDLK_9:
				case SDLK_a:
				case SDLK_b:
				case SDLK_c:
				case SDLK_d:
				case SDLK_e:
				case SDLK_f:
					aBuf_[aBufPos_] = key_[e.key.keysym.sym];
					if(aBufPos_ < 3) ++aBufPos_;
					break;
			}
			break;
	}

	if(state_ == State::GOTO)
	{
		setGotoCursor();
	}
	else
	{
		cx_ = cy_ = -1;
	}
}

void RAMMonitor::onEventEditing(const SDL_Event& e)
{
	if(window_.hasFocus()) switch(e.type)
	{
		case SDL_KEYDOWN:
			switch(e.key.keysym.sym)
			{
				case SDLK_0:
				case SDLK_1:
				case SDLK_2:
				case SDLK_3:
				case SDLK_4:
				case SDLK_5:
				case SDLK_6:
				case SDLK_7:
				case SDLK_8:
				case SDLK_9:
				case SDLK_a:
				case SDLK_b:
				case SDLK_c:
				case SDLK_d:
				case SDLK_e:
				case SDLK_f:
					access_(addr_) = (access_(addr_) & 0xF0) | key_[e.key.keysym.sym];
					push(undo_, addr_, buf_, access_(addr_));
					gotoAddr(addr_ + 1);
					state_ = State::DEFAULT;
					break;
				case SDLK_ESCAPE:
					access_(addr_) = buf_;
					state_ = State::DEFAULT;
					break;
			}
			break;
	}

	if(state_ != State::EDITING)
	{
		cx_ = cy_ = -1;
	}
}

std::string RAMMonitor::renderDefault(void)
{
	return lib::stringf("$%04X: 0x%02X", addr_, access_(addr_));
}

std::string RAMMonitor::renderGoto(void)
{
	return lib::stringf("GOTO $%X%X%X%X", aBuf_[0], aBuf_[1], aBuf_[2], aBuf_[3]);
}

std::string RAMMonitor::renderEditing(void)
{
	return "";
}

void RAMMonitor::click(int x, int y)
{
	int cx = (x / CHAR_W) - 7, cy = (y / CHAR_H) - 2;

	if(cx < 0 || cy < 0 || (cx % 3) == 2 || cy >= (viewsize_ / 0x10))
		return;
	
	cx /= 3;

	addr_ = vOff_ + cx + cy * 0x10;
}

void RAMMonitor::scroll(int dy)
{
	if(dy < 0) while(dy < 0)
	{
		if(vOff_ < 0x10) break;

		if((vOff_ -= 0x10) + viewsize_ <= addr_)
		{
			addr_ -= 0x10;
		}

		++dy;
	}
	else while(dy > 0)
	{
		if(vOff_ + viewsize_ >= 0x10000) break;

		if((vOff_ += 0x10) > addr_)
		{
			addr_ += 0x10;
		}

		--dy;
	}
}

void RAMMonitor::push(stack_t& s, uint16_t a, uint8_t o, uint8_t n)
{
	Change c;

	c.addr = a;
	c.prev = o;
	c.next = n;

	s.push(c);
}

void RAMMonitor::act(stack_t& s1, stack_t& s2)
{
	if(s1.empty()) return;

	Change c(s1.top());

	gotoAddr(c.addr);

	access_(addr_) = c.prev;

	push(s2, c.addr, c.next, c.prev);
}

void RAMMonitor::gotoAddr(uint16_t addr)
{
	addr_ = addr;

	if(addr_ < vOff_ || addr_ > vOff_ - 1 + viewsize_)
	{
		vOff_ = ((addr_ >= viewsize_ / 2) ? addr_ - viewsize_ / 2 : 0) & ~0xf;
		if(((vOff_ + viewsize_) & 0xFFFF) < vOff_) vOff_ = (0x10000 - viewsize_) & ~0xf;
	}
}

void RAMMonitor::setGotoCursor(void)
{
	cx_ = 6 + aBufPos_;
	cy_ = 2 + viewsize_ / 0x10 + 1;
}

}

