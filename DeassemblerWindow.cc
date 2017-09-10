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

namespace z80 {

using winui::Space;
using winui::Position;
using lib::Character;

DeassemblerWindow::DeassemblerWindow(Z80& cpu, uint lc)
	: window_(WIN_TITLE, Space(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W*CHAR_W, (WIN_H+lc)*CHAR_H))
	, charset_(CHARSET_PATH)
	, cLines_(lc)
	, cpu_(&cpu)
{
	window_.onUpdate([this](uint ms) { onUpdate(ms); });
	window_.onRender([this]( ) { onRender(); });
	window_.onEvent([this](const SDL_Event& e) { onEvent(e); });

	addr_ = 0;
}

DeassemblerWindow::~DeassemblerWindow(void)
{
}

void DeassemblerWindow::onUpdate(uint ms)
{
}

void DeassemblerWindow::onRender(void)
{
	if(followPC_) addr_ = cpu_->getPC();

	uint16_t a = addr_;
	static const std::string header(MXT_HEADER);

	window_.clear();

	for(uint x = 0 ; x < WIN_W ; ++x)
	{
		if(x == MXT_VLINE)
		{
			drawChar(MXT_VLINE, 0, Character::S_UD);
			drawChar(MXT_VLINE, 1, Character::S_UD_H_LR);
			drawChar(MXT_VLINE, 2 + cLines_, Character::S_U_H_LR);
		}
		else
		{
			drawChar(x, 0, x >= header.size() ? ' ' : header.at(x));
			drawChar(x, 1, Character::H_LR);
			drawChar(x, 2 + cLines_, Character::H_LR);
		}
	}

	for(uint i = 0 ; i < cLines_ ; ++i)
	{
		Instruction de(disassemble(&cpu_->RAM(a)));
		renderLine(i + 2, a, de);
		a += de.size;
	}
}

void DeassemblerWindow::onEvent(const SDL_Event& e)
{
}

void DeassemblerWindow::drawChar(uint x, uint y, uint8_t ch, uint c, bool inv)
{
	Space s(ch * CHAR_W, (c + (inv ? CHAR_COLORSPACE : 0)) * CHAR_H, CHAR_W, CHAR_H);

	window_.draw(charset_.region(s), Position(x * CHAR_W, y * CHAR_H));
}

void DeassemblerWindow::renderLine(uint y, uint16_t addr, const Instruction& ins)
{
	bool isPC = cpu_->getPC() >= addr && cpu_->getPC() < addr + ins.size;
	uint c = isPC ? (followPC_ ? COLOR_RED : COLOR_CYAN) : COLOR_BLACK;

	std::string raw;

	for(uint i = 0 ; i < ins.size ; ++i)
	{
		raw += lib::stringf("%02X ", cpu_->RAM(addr + i));
	}

	std::string s(lib::stringf("$%04X | %- 11s %s", addr, raw.c_str(), ins.literal.c_str()));

	for(uint x = 0 ; x < WIN_W ; ++x)
	{
		drawChar(x, y, x == MXT_VLINE ? Character::S_UD : (x >= s.size() ? ' ' : s.at(x)), c, isPC);
	}
}

}
