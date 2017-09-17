#include "DeassemblerWindow.h"

#define MXT_HEADER " ADDR |             Instruction"
#define MXT_VLINE 6

#define WIN_TITLE "Z80 Disassembler"
#define WIN_W (5+3+(4*2+3)+1+24)
#define WIN_H (2+2)

#define CHARSET_PATH "charset.bmp"
#define CHAR_W 8
#define CHAR_H 12
#define CHAR_COLORSPACE 8

#define MXT_ERROR "ERR"

namespace z80 {

using winui::Space;
using winui::Position;
using winui::Dimension;
using winui::Image;
using winui::Color;
using winui::CharacterWindow;
using lib::Character;

DeassemblerWindow::DeassemblerWindow(Z80& cpu, uint lc)
	: CharacterWindow(WIN_TITLE, Position::CENTER(), Dimension(WIN_W, WIN_H + lc), Image(CHARSET_PATH), Dimension(CHAR_W, CHAR_H), CHAR_COLORSPACE)
	, cLines_(lc)
	, cpu_(&cpu)
{
	pc_ = sel_ = addr_ = 0;
	followPC_ = false;
	off_ = 0;

	state_ = State::DEFAULT;

	clearBuf();

	enableBlink(false);
	setDefaultColor(Color::WHITE());

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
}

DeassemblerWindow::~DeassemblerWindow(void)
{
}

void DeassemblerWindow::onUpdate(uint ms)
{
}

void DeassemblerWindow::onRender(void)
{
	if(followPC_ && pc_ != cpu_->getPC())
	{
		pc_ = addr_ = cpu_->getPC();
		off_ = 0;
	}

	uint a = addr_;
	static const std::string header(MXT_HEADER);
	std::string fooder(followPC_ ? "Following PC" : "");

	clear();
	addresses_.clear();

	for(uint x = 0 ; x < WIN_W ; ++x)
	{
		if(x == MXT_VLINE)
		{
			renderChar(Position(MXT_VLINE, 0), Character::S_UD);
			renderChar(Position(MXT_VLINE, 1), Character::S_UD_H_LR);
			renderChar(Position(MXT_VLINE, 2 + cLines_), Character::S_U_H_LR);
		}
		else
		{
			renderChar(Position(x, 0), x >= header.size() ? ' ' : header.at(x));
			renderChar(Position(x, 1), Character::H_LR);
			renderChar(Position(x, 2 + cLines_), Character::H_LR);
		}

		renderChar(Position(x, 3 + cLines_), x >= fooder.size() ? ' ' : fooder.at(x));
	}

	for(uint i = 0 ; i < cLines_ + off_ ; ++i)
	{
		if(a >= 0x10000)
		{
			if(i >= off_) renderEmptyLine(i - off_ + 2);
		}
		else
		{
			Instruction de = disassemble(&cpu_->RAM(a));
			if(i >= off_) renderLine(i - off_ + 2, a, de);
			a += de.size;
		}
	}

	scrollable_ = a < 0x10000;

	renderString(Position(0, 2 + cLines_ + 1), getFooder());
}

void DeassemblerWindow::onEvent(const SDL_Event& e)
{
	switch(state_)
	{
		case State::DEFAULT:
			onEventDefault(e);
			break;
		case State::GOTO:
			onEventGoto(e);
			break;
	}
}

void DeassemblerWindow::onEventGoto(const SDL_Event& e)
{
	if(hasFocus()) switch(e.type)
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
					clearBuf();
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
		enableBlink(false);
	}
}

void DeassemblerWindow::onEventDefault(const SDL_Event& e)
{
	if(hasFocus()) switch(e.type)
	{
		case SDL_KEYDOWN:
			switch(e.key.keysym.sym)
			{
				case SDLK_f:
					if((followPC_ = !followPC_))
					{
						pc_ = addr_ = cpu_->getPC();
					}
					break;
				case SDLK_g:
					state_ = State::GOTO;
					setGotoCursor();
					clearBuf();
					break;
			}
	}

	if(isMouseOver()) switch(e.type)
	{
		case SDL_MOUSEBUTTONDOWN:
			switch(e.button.button)
			{
				case SDL_BUTTON_LEFT:
					if(e.button.clicks >= 2)
					{
						click(e.button.y, true);
					}
					break;
			}
			break;
		case SDL_MOUSEWHEEL:
			scroll((SDL_GetModState() & KMOD_SHIFT ? 10 : 1) * 
				e.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? -e.wheel.y : e.wheel.y);
			break;
		case SDL_MOUSEMOTION:
			click(e.motion.y, false);
			break;
	}
}

std::string DeassemblerWindow::getFooder(void)
{
	switch(state_)
	{
		case State::DEFAULT:
			return "";
		case State::GOTO:
			return lib::stringf("GOTO $%X%X%X%X", aBuf_[0], aBuf_[1], aBuf_[2], aBuf_[3]);
	}

	return "";
}

void DeassemblerWindow::renderLine(uint y, uint16_t addr, const Instruction& ins)
{
	bool isPC = pc_ >= addr && pc_ < addr + ins.size;
	bool isSel = sel_ == y;
	uint c = isPC ? COLOR_RED : isSel ? COLOR_CYAN : COLOR_BLACK;

	addresses_[y] = addr;

	std::string raw;

	for(uint i = 0 ; i < ins.size ; ++i)
	{
		raw += lib::stringf("%02X ", cpu_->RAM(addr + i));
	}

	std::string s(lib::stringf(
		"$%04X%c| %- 12s %s", addr, checkBreak_(addr) ? '*' : ' ', raw.c_str(), ins.literal.c_str()));

	for(uint x = 0 ; x < WIN_W ; ++x)
	{
		renderChar(Position(x, y), x == MXT_VLINE ? Character::S_UD : (x >= s.size() ? ' ' : s.at(x)), c, isPC || isSel);
	}
}

void DeassemblerWindow::renderEmptyLine(uint y)
{
	bool isSel = sel_ == y;
	uint c = isSel ? COLOR_CYAN : COLOR_BLACK;

	for(uint x = 0 ; x < WIN_W ; ++x)
	{
		renderChar(Position(x, y), x == MXT_VLINE ? Character::S_UD : ' ', c, isSel);
	}
}

void DeassemblerWindow::scroll(int dy)
{
	while(dy < 0)
	{
		if(off_ > 0) --off_;
		++dy;
	}
	while(dy > 0)
	{
		if(scrollable_) ++off_;
		--dy;
	}
}

void DeassemblerWindow::click(int y, bool f)
{
	if(y < 0) return;

	y /= CHAR_H;

	if(y < 2 || (uint)y >= 2 + cLines_) return;

	if(f)
	{
		auto i = addresses_.find(y);

		if(i == addresses_.end()) return;

		setBreak_(i->second);
	}
	else
	{
		sel_ = y;
	}
}

void DeassemblerWindow::setGotoCursor(void)
{
	enableBlink(true);
	updateCursor(Position(6 + aBufPos_, 2 + cLines_ + 1));
}

void DeassemblerWindow::gotoAddr(uint16_t a)
{
	addr_ = a;
	sel_ = a;
	off_ = 0;
}

}

