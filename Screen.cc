#include "Screen.h"

namespace z80 {

Screen::Screen(void)
{
	timerEn_ = false;
}

Screen::~Screen(void)
{
}

void Screen::write(uint8_t port, uint8_t data)
{
	switch(port)
	{
		case 0x00: // data port
			writeToVRAM(data);
			break;
		case 0x01: // cursor x
			cx_ = data;
			break;
		case 0x02: // cursor y
			cy_ = data;
			break;
		case 0x03: // status
			if(data & 0x80)
				command(data & 0x7F);
			else
				status_ = data;
			break;
		case 0x0F: // ID
			break;
	}
}

uint8_t Screen::read(uint8_t port)
{
	switch(port)
	{
		case 0x00: // data port
			return vram_[cx_ + cy_ * COLS];
		case 0x01: // cursor x
			return cx_;
		case 0x02: // cursor y
			return cy_;
		case 0x03: // status
			return status_;
		case 0x0F: // ID
			return id_++ & 1 ? SCREEN_ID : Peripheral::PER_ID;
	}

	return 0;
}

void Screen::command(uint8_t data)
{
	switch(data)
	{
		case 0x00: // clear screen
			for(uint i = 0 ; i < COLS*ROWS ; ++i)
			{
				vram_[i] = 0;
			}
			cx_ = cy_ = 0;
			break;
		case 0x01: // scroll
			do_scroll();
			if(cy_ > 0) --cy_;
			break;
		case 0x02: // enable timer
			timerEn_ = true;
			break;
		case 0x03: // disable timer
			timerEn_ = false;
			break;
	}
}

void Screen::do_scroll(void)
{
	for(uint i = 1 ; i < ROWS ; ++i)
	{
		for(uint j = 0 ; j < COLS ; ++j)
		{
			vram_[j + (i - 1) * COLS] = vram_[j + i * COLS];
		}
	}

	for(uint i = 0 ; i < COLS; ++i)
	{
		vram_[i + (ROWS - 1) * COLS] = 0;
	}
}

void Screen::writeToVRAM(uint8_t data)
{
	switch(data)
	{
		case 8: // backspace
			if(cx_ > 0) --cx_;
			break;
		case 13: // newline
			cx_ = 0;
			if(cy_ < ROWS-1)
				++cy_;
			else
				do_scroll();
			break;
		case 17: // left
			if(cx_ > 0) --cx_;
			break;
		case 18: // right
			if(cx_ < COLS-1) ++cx_;
			break;
		case 19: // up
			if(cy_ > 0) --cy_;
			break;
		case 20: // down
			if(cy_ < ROWS-1) ++cy_;
			break;
		default:
			vram_[cx_ + cy_ * COLS] = data;
			if(advX())
			{
				if(cx_ >= COLS-1)
				{
					if(wrapX())
						cx_ = 0;

					if(advY())
					{
						if(cy_ >= ROWS-1)
						{
							if(scroll_en())
							{
								do_scroll();
							}
						}
						else
						{
							++cy_;
						}
					}
				}
				else
				{
					++cx_;
				}
			}
			break;
	}
}

}

