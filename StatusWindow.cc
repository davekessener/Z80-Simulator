#include <vector>

#include "StatusWindow.h"
#include "lib.h"

#define WIN_TITLE "Z80 Status"
#define WIN_W 41
#define WIN_H 14

#define CHARSET_PATH "charset.bmp"
#define CHAR_W 8
#define CHAR_H 12

#define CHAR_SOLID 128
#define CHAR_HOLLOW 139
#define CHAR_MIX 150
#define CHAR_S_UD     (CHAR_SOLID+1)
#define CHAR_S_LR     (CHAR_SOLID+0)
#define CHAR_S_LRUD   (CHAR_SOLID+2)
#define CHAR_H_RD     (CHAR_HOLLOW+5)
#define CHAR_H_LR     (CHAR_HOLLOW+0)
#define CHAR_H_LD     (CHAR_HOLLOW+6)
#define CHAR_H_UD     (CHAR_HOLLOW+1)
#define CHAR_H_RUD    (CHAR_HOLLOW+10)
#define CHAR_H_LUD    (CHAR_HOLLOW+9)
#define CHAR_H_LRD    (CHAR_HOLLOW+8)
#define CHAR_H_LRU    (CHAR_HOLLOW+7)
#define CHAR_H_LRUD   (CHAR_HOLLOW+2)
#define CHAR_H_RU     (CHAR_HOLLOW+4)
#define CHAR_H_LU     (CHAR_HOLLOW+3)
#define CHAR_H_LR_S_D (CHAR_MIX+5)
#define CHAR_H_LR_S_U (CHAR_MIX+4)
#define CHAR_H_UD_S_R (CHAR_MIX+7)
#define CHAR_H_UD_S_L (CHAR_MIX+6)
#define CHAR_H_UD_S_LR (CHAR_MIX+8)

#define COLOR_BLACK 0
#define COLOR_GREEN 2

// +---+---------------+----+--------------+
// |PC | $XXXX (YYYYY) | SP | $XXXX (YYYYY)|
// |AF | $XXXX (YYYYY) | AF'| $XXXX (YYYYY)|
// |BC | $XXXX (YYYYY) | BC'| $XXXX (YYYYY)|
// |DE | $XXXX (YYYYY) | DE'| $XXXX (YYYYY)|
// |HL | $XXXX (YYYYY) | HL'| $XXXX (YYYYY)|
// |IX | $XXXX (YYYYY) | IY | $XXXX (YYYYY)|
// +---------------------------------------+
// | @$XXXX: INSTRUCTION                   |
// +---------------------------------------+
// |Flags  | S | Z |   | H |P/V|   | N | C |
// +---------------------------------------+
// |       | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
// +---------------------------------------+

namespace z80 {

using winui::Space;
using winui::Position;
using winui::Color;

StatusWindow::StatusWindow(Z80& cpu)
	: window_(WIN_TITLE, Space(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIN_W*CHAR_W, WIN_H*CHAR_H))
	, charset_(CHARSET_PATH)
	, cpu_(&cpu)
{
	window_.setDefaultColor(Color::WHITE());

	window_.onUpdate([this](uint ms) { onUpdate(ms); });
	window_.onRender([this]( ) { onRender(); });
	window_.onEvent([this](const SDL_Event& e) { onEvent(e); });
}

void StatusWindow::onUpdate(uint ms)
{
}

void StatusWindow::onRender(void)
{
#define ADD(x) screen.push_back(x)
	std::vector<uint> screen;

	auto printS = [&screen](const std::string& s, uint min)
	{
		for(uint i = 0 ; i < min ; ++i)
		{
			ADD(i < s.size() ? s[i] : ' ');
		}
	};

	auto printR = [&screen, &printS](const std::string& r, uint16_t v)
	{
		printS(r, 3);
		ADD(CHAR_S_UD);
		printS(lib::stringf(" $%04X (% 5s)", v, lib::stringf("%u", v).c_str()), 14);
	};

	auto printN = [&screen](uint v, uint n)
	{
		for(uint i = 0 ; i < n ; ++i)
		{
			ADD(v);
		}
	};

	if(screen.size() != 0*WIN_W) throw lib::stringf("%d:%u", 0, screen.size());

	ADD(CHAR_H_RD);
	printN(CHAR_H_LR, 3);
	ADD(CHAR_H_LR_S_D);
	printN(CHAR_H_LR, 15);
	ADD(CHAR_H_LR_S_D);
	printN(CHAR_H_LR, 4);
	ADD(CHAR_H_LR_S_D);
	printN(CHAR_H_LR, 14);
	ADD(CHAR_H_LD);

	if(screen.size() != 1*WIN_W) throw lib::stringf("%d:%u", 1, screen.size());

	ADD(CHAR_H_UD);
	printR("PC", cpu_->getPC());
	ADD(' ');
	ADD(CHAR_S_UD);
	ADD(' ');
	printR("SP", cpu_->getSP());
	ADD(CHAR_H_UD);

	if(screen.size() != 2*WIN_W) throw lib::stringf("%d:%u", 2, screen.size());

	ADD(CHAR_H_UD);
	printR("AF", cpu_->getAF());
	ADD(' ');
	ADD(CHAR_S_UD);
	ADD(' ');
	printR("AF'", cpu_->getAFp());
	ADD(CHAR_H_UD);

	if(screen.size() != 3*WIN_W) throw lib::stringf("%d:%u", 3, screen.size());

	ADD(CHAR_H_UD);
	printR("BC", cpu_->getBC());
	ADD(' ');
	ADD(CHAR_S_UD);
	ADD(' ');
	printR("BC'", cpu_->getBCp());
	ADD(CHAR_H_UD);

	if(screen.size() != 4*WIN_W) throw lib::stringf("%d:%u", 4, screen.size());

	ADD(CHAR_H_UD);
	printR("DE", cpu_->getDE());
	ADD(' ');
	ADD(CHAR_S_UD);
	ADD(' ');
	printR("DE'", cpu_->getDEp());
	ADD(CHAR_H_UD);

	if(screen.size() != 5*WIN_W) throw lib::stringf("%d:%u", 5, screen.size());

	ADD(CHAR_H_UD);
	printR("HL", cpu_->getHL());
	ADD(' ');
	ADD(CHAR_S_UD);
	ADD(' ');
	printR("HL'", cpu_->getHLp());
	ADD(CHAR_H_UD);

	if(screen.size() != 6*WIN_W) throw lib::stringf("%d:%u", 6, screen.size());

	ADD(CHAR_H_UD);
	printR("IX", cpu_->getIX());
	ADD(' ');
	ADD(CHAR_S_UD);
	ADD(' ');
	printR("IY", cpu_->getIY());
	ADD(CHAR_H_UD);

	if(screen.size() != 7*WIN_W) throw lib::stringf("%d:%u", 7, screen.size());

	ADD(CHAR_H_RUD);
	printN(CHAR_H_LR, 3);
	ADD(CHAR_H_LR_S_U);
	printN(CHAR_H_LR, 15);
	ADD(CHAR_H_LR_S_U);
	printN(CHAR_H_LR, 4);
	ADD(CHAR_H_LR_S_U);
	printN(CHAR_H_LR, 14);
	ADD(CHAR_H_LUD);

	if(screen.size() != 8*WIN_W) throw lib::stringf("%d:%u", 8, screen.size());

	ADD(CHAR_H_UD);
	printS(lib::stringf(" @$%04X: %s", cpu_->getPC(), cpu_->disassemble(cpu_->getPC()).c_str()), WIN_W-2);
	ADD(CHAR_H_UD);

	if(screen.size() != 9*WIN_W) throw lib::stringf("%d:%u", 9, screen.size());

	ADD(CHAR_H_RUD);
	printN(CHAR_H_LR, 7);
	ADD(CHAR_H_LRD);
	for(uint i = 0 ; i < 8 ; ++i)
	{
		printN(CHAR_H_LR, 3);
		ADD(i == 7 ? CHAR_H_LUD : CHAR_H_LR_S_D);
	}

	if(screen.size() != 10*WIN_W) throw lib::stringf("%d:%u", 10, screen.size());

	ADD(CHAR_H_UD);
	printS("", 7);
	ADD(CHAR_H_UD);
	printS(" S ", 3);
	ADD(CHAR_S_UD);
	printS(" Z ", 3);
	ADD(CHAR_S_UD);
	printS("   ", 3);
	ADD(CHAR_S_UD);
	printS(" H ", 3);
	ADD(CHAR_S_UD);
	printS("P/V", 3);
	ADD(CHAR_S_UD);
	printS("   ", 3);
	ADD(CHAR_S_UD);
	printS(" N ", 3);
	ADD(CHAR_S_UD);
	printS(" C ", 3);
	ADD(CHAR_H_UD);

	if(screen.size() != 11*WIN_W) throw lib::stringf("%d:%u", 11, screen.size());

	ADD(CHAR_H_UD);
	printS(" Flags", 7);
	ADD(CHAR_H_UD_S_R);
	for(uint i = 0 ; i < 8 ; ++i)
	{
		printN(CHAR_S_LR, 3);
		ADD(i == 7 ? CHAR_H_UD_S_L : CHAR_S_LRUD);
	}

	if(screen.size() != 12*WIN_W) throw lib::stringf("%d:%u", 12, screen.size());

	ADD(CHAR_H_UD);
	printS("", 7);
	ADD(CHAR_H_UD);
	printS(lib::stringf(" %d ", cpu_->getFlagS()), 3);
	ADD(CHAR_S_UD);
	printS(lib::stringf(" %d ", cpu_->getFlagZ()), 3);
	ADD(CHAR_S_UD);
	printS(lib::stringf(" %d ", 0), 3);
	ADD(CHAR_S_UD);
	printS(lib::stringf(" %d ", cpu_->getFlagH()), 3);
	ADD(CHAR_S_UD);
	printS(lib::stringf(" %d ", cpu_->getFlagPV()), 3);
	ADD(CHAR_S_UD);
	printS(lib::stringf(" %d ", 0), 3);
	ADD(CHAR_S_UD);
	printS(lib::stringf(" %d ", cpu_->getFlagN()), 3);
	ADD(CHAR_S_UD);
	printS(lib::stringf(" %d ", cpu_->getFlagC()), 3);
	ADD(CHAR_H_UD);

	if(screen.size() != 13*WIN_W) throw lib::stringf("%d:%u", 13, screen.size());

	ADD(CHAR_H_RU);
	printN(CHAR_H_LR, 7);
	ADD(CHAR_H_LRU);
	for(uint i = 0 ; i < 8 ; ++i)
	{
		printN(CHAR_H_LR, 3);
		ADD(i == 7 ? CHAR_H_LU : CHAR_H_LR_S_U);
	}

	if(screen.size() != WIN_H*WIN_W)
	{
		throw lib::stringf("u done goofed %u", screen.size());
	}

	window_.clear();

	for(uint y = 0 ; y < WIN_H ; ++y)
	{
		for(uint x = 0 ; x < WIN_W ; ++x)
		{
			uint c = screen[x + y * WIN_W];
			Space s(c * CHAR_W, COLOR_BLACK * CHAR_H, CHAR_W, CHAR_H);
			window_.draw(charset_.region(s), Position(x * CHAR_W, y * CHAR_H));
		}
	}

#undef ADD
}

void StatusWindow::onEvent(const SDL_Event& e)
{
}

}
