#include <SDL.h>

#include "Keyboard.h"

#define MXT_KEY_EXTENDED 0xE0
#define MXT_KEY_BREAK 0xF0

#define MXT_SHIFT 0x01
#define MXT_CTRL 0x02
#define MXT_ALT 0x04

#define KEY_PUP 0x80
#define KEY_PDOWN 0x81
#define KEY_PAUSE 0x82
#define KEY_HOME 0x83
#define KEY_INSERT 0x84
#define KEY_BACKSPACE 0x08
#define KEY_ESCAPE 0x1B
#define KEY_DELETE 0x85
#define KEY_UP 0x86
#define KEY_DOWN 0x87
#define KEY_LEFT 0x88
#define KEY_RIGHT 0x89
#define KEY_END 0x8A
#define KEY_F1 0x8B
#define KEY_F2 0x8C
#define KEY_F3 0x8D
#define KEY_F4 0x8E
#define KEY_F5 0x8F
#define KEY_F6 0x90
#define KEY_F7 0x91
#define KEY_F8 0x92
#define KEY_F9 0x93
#define KEY_F10 0x94
#define KEY_F11 0x95
#define KEY_F12 0x96

namespace z80 {

std::map<uint, uint8_t> Keyboard::mSimple;
std::map<uint, uint8_t> Keyboard::mExtended;
std::map<uint, uint8_t> Keyboard::mASCII;
std::map<uint, uint8_t> Keyboard::mASCIIshifted;

Keyboard::Keyboard(void)
	: intLine_(new bool)
{
    static bool initialized = false;

	reset();

	if(!initialized)
	{
	    init();
		initialized = false;
	}
}

void Keyboard::press(uint key, bool down)
{
	if(mode_ == MODE_RAW)
	{
		auto i = mSimple.find(key);

		if(i != mSimple.end())
		{
		    if(!down) buf_.push_back(MXT_KEY_BREAK);
			buf_.push_back(i->second);
			intLine_.set(true);
		}
		else if((i = mExtended.find(key)) != mExtended.end())
		{
		    buf_.push_back(MXT_KEY_EXTENDED);
			if(!down) buf_.push_back(MXT_KEY_BREAK);
			buf_.push_back(i->second);
			intLine_.set(true);
		}
	}
	else if(mode_ == MODE_POLL || mode_ == MODE_TEXT)
	{
		if(key == SDLK_LSHIFT || key == SDLK_RSHIFT)
		{
			shift_ = down;
		}
		else if(key == SDLK_LCTRL || key == SDLK_RCTRL)
		{
			ctrl_ = down;
		}
		else if(key == SDLK_LALT || key == SDLK_RALT)
		{
			alt_ = down;
		}
		else
		{
			uint8_t aKey = getASCII(key);

			if(aKey != 0)
			{
				if(mode_ == MODE_TEXT)
				{
					if(down)
					{
						buf_.push_back(aKey);
						intLine_.set(true);
					}
				}
				else if(mode_ == MODE_POLL)
				{
					if(down)
					{
						pressed_.push_back(aKey);
					}
					else
					{
						auto i = std::find(pressed_.begin(), pressed_.end(), aKey);
						if(i != pressed_.end()) pressed_.erase(i);
					}
				}
			}
		}
	}
}

void Keyboard::write(uint8_t port, uint8_t data)
{
	switch(port)
	{
		case 0x00:
			mode_ = data;
			break;
		case 0x01:
			if(mode_ == MODE_POLL)
			{
				poll_ = data;
			}
			break;
	}
}

uint8_t Keyboard::read(uint8_t port)
{
    uint8_t r = 0x00;

    switch(port)
	{
	    case 0x00:
		    r = buf_.size();
			break;
		case 0x01:
			if(mode_ == MODE_POLL)
			{
				r =   poll_ == 0 
					? (pressed_.empty() ? 1 : 0) 
					: (std::find(pressed_.begin(), pressed_.end(), poll_) == pressed_.end() ? 0 : 1);
			}
			else
			{
				if(!buf_.empty())
				{
		    		r = buf_.front();
					buf_.pop_front();
				}
			}
			break;
		case 0x02:
			r = (shift_ ? MXT_SHIFT : 0) | (ctrl_ ? MXT_CTRL : 0) | (alt_ ? MXT_ALT : 0);
			break;
		case 0x0F:
			break;
	}

	return r;
}

uint8_t Keyboard::getASCII(uint key) const
{
	auto i = mASCIIshifted.find(key);

	if(i == mASCIIshifted.end() || !shift_)
	{
		i = mASCII.find(key);
	}

	return i == mASCII.end() ? 0 : i->second;
}

void Keyboard::reset(void)
{
	buf_.clear();
	pressed_.clear();
	mode_ = MODE_POLL;
	shift_ = ctrl_ = alt_ = false;
	intLine_.set(false);
}

void Keyboard::init(void)
{
    mSimple[SDLK_a] = 0x1C;
	mSimple[SDLK_b] = 0x32;
	mSimple[SDLK_c] = 0x21;
	mSimple[SDLK_d] = 0x23;
	mSimple[SDLK_e] = 0x24;
	mSimple[SDLK_f] = 0x2B;
	mSimple[SDLK_g] = 0x34;
	mSimple[SDLK_h] = 0x33;
	mSimple[SDLK_i] = 0x43;
	mSimple[SDLK_j] = 0x3B;
	mSimple[SDLK_k] = 0x42;
	mSimple[SDLK_l] = 0x4B;
	mSimple[SDLK_m] = 0x3A;
	mSimple[SDLK_n] = 0x31;
	mSimple[SDLK_o] = 0x44;
	mSimple[SDLK_p] = 0x4D;
	mSimple[SDLK_q] = 0x15;
	mSimple[SDLK_r] = 0x2D;
	mSimple[SDLK_s] = 0x1B;
	mSimple[SDLK_t] = 0x2C;
	mSimple[SDLK_u] = 0x3C;
	mSimple[SDLK_v] = 0x2A;
	mSimple[SDLK_w] = 0x1D;
	mSimple[SDLK_x] = 0x22;
	mSimple[SDLK_y] = 0x35;
	mSimple[SDLK_z] = 0x1A;
	mSimple[SDLK_0] = 0x45;
	mSimple[SDLK_1] = 0x16;
	mSimple[SDLK_2] = 0x1E;
	mSimple[SDLK_3] = 0x26;
	mSimple[SDLK_4] = 0x25;
	mSimple[SDLK_5] = 0x2E;
	mSimple[SDLK_6] = 0x36;
	mSimple[SDLK_7] = 0x3D;
	mSimple[SDLK_8] = 0x3E;
	mSimple[SDLK_9] = 0x46;
	mSimple[SDLK_QUOTE] = 0x52;
	mSimple[SDLK_BACKQUOTE] = 0x0E;
	mSimple[SDLK_MINUS] = 0x4E;
	mSimple[SDLK_EQUALS] = 0x55;
	mSimple[SDLK_BACKSLASH] = 0x5D;
	mSimple[SDLK_BACKSPACE] = 0x66;
	mSimple[SDLK_SPACE] = 0x29;
	mSimple[SDLK_TAB] = 0x0D;
	mSimple[SDLK_CAPSLOCK] = 0x58;
	mSimple[SDLK_LSHIFT] = 0x12;
	mSimple[SDLK_LCTRL] = 0x14;
	mSimple[SDLK_LALT] = 0x11;
	mSimple[SDLK_RSHIFT] = 0x59;
	mSimple[SDLK_RETURN] = 0x5A;
	mSimple[SDLK_ESCAPE] = 0x76;
	mSimple[SDLK_F1] = 0x05;
	mSimple[SDLK_F2] = 0x06;
	mSimple[SDLK_F3] = 0x04;
	mSimple[SDLK_F4] = 0x0C;
	mSimple[SDLK_F5] = 0x03;
	mSimple[SDLK_F6] = 0x0B;
	mSimple[SDLK_F7] = 0x83;
	mSimple[SDLK_F8] = 0x0A;
	mSimple[SDLK_F9] = 0x01;
	mSimple[SDLK_F10] = 0x09;
	mSimple[SDLK_F11] = 0x78;
	mSimple[SDLK_F12] = 0x07;
	mSimple[SDLK_LEFTBRACKET] = 0x54;
	mSimple[SDLK_NUMLOCKCLEAR] = 0x77;
	mSimple[SDLK_KP_MULTIPLY] = 0x7C;
	mSimple[SDLK_KP_MINUS] = 0x7B;
	mSimple[SDLK_KP_PLUS] = 0x79;
	mSimple[SDLK_KP_PERIOD] = 0x71;
	mSimple[SDLK_KP_0] = 0x70;
	mSimple[SDLK_KP_1] = 0x69;
	mSimple[SDLK_KP_2] = 0x72;
	mSimple[SDLK_KP_3] = 0x7A;
	mSimple[SDLK_KP_4] = 0x6B;
	mSimple[SDLK_KP_5] = 0x73;
	mSimple[SDLK_KP_6] = 0x74;
	mSimple[SDLK_KP_7] = 0x6C;
	mSimple[SDLK_KP_8] = 0x75;
	mSimple[SDLK_KP_9] = 0x7D;
	mSimple[SDLK_RIGHTBRACKET] = 0x5B;
	mSimple[SDLK_SEMICOLON] = 0x4C;
	mSimple[SDLK_COMMA] = 0x41;
	mSimple[SDLK_PERIOD] = 0x49;
	mSimple[SDLK_SLASH] = 0x4A;

    mExtended[SDLK_RCTRL] = 0x14;
	mExtended[SDLK_RALT] = 0x11;
	mExtended[SDLK_INSERT] = 0x70;
	mExtended[SDLK_HOME] = 0x6C;
	mExtended[SDLK_PAGEUP] = 0x7D;
	mExtended[SDLK_DELETE] = 0x71;
	mExtended[SDLK_END] = 0x69;
	mExtended[SDLK_PAGEDOWN] = 0x7A;
	mExtended[SDLK_UP] = 0x75;
	mExtended[SDLK_LEFT] = 0x6B;
	mExtended[SDLK_DOWN] = 0x72;
	mExtended[SDLK_RIGHT] = 0x74;
	mExtended[SDLK_KP_DIVIDE] = 0x4A;

	mASCIIshifted[SDLK_a] = 'A';
	mASCIIshifted[SDLK_b] = 'B';
	mASCIIshifted[SDLK_c] = 'C';
	mASCIIshifted[SDLK_d] = 'D';
	mASCIIshifted[SDLK_e] = 'E';
	mASCIIshifted[SDLK_f] = 'F';
	mASCIIshifted[SDLK_g] = 'G';
	mASCIIshifted[SDLK_h] = 'H';
	mASCIIshifted[SDLK_i] = 'I';
	mASCIIshifted[SDLK_j] = 'J';
	mASCIIshifted[SDLK_k] = 'K';
	mASCIIshifted[SDLK_l] = 'L';
	mASCIIshifted[SDLK_m] = 'M';
	mASCIIshifted[SDLK_n] = 'N';
	mASCIIshifted[SDLK_o] = 'O';
	mASCIIshifted[SDLK_p] = 'P';
	mASCIIshifted[SDLK_q] = 'Q';
	mASCIIshifted[SDLK_r] = 'R';
	mASCIIshifted[SDLK_s] = 'S';
	mASCIIshifted[SDLK_t] = 'T';
	mASCIIshifted[SDLK_u] = 'U';
	mASCIIshifted[SDLK_v] = 'V';
	mASCIIshifted[SDLK_w] = 'W';
	mASCIIshifted[SDLK_x] = 'X';
	mASCIIshifted[SDLK_y] = 'Y';
	mASCIIshifted[SDLK_z] = 'Z';
	mASCIIshifted[SDLK_0] = ')';
	mASCIIshifted[SDLK_1] = '!';
	mASCIIshifted[SDLK_2] = '@';
	mASCIIshifted[SDLK_3] = '#';
	mASCIIshifted[SDLK_4] = '$';
	mASCIIshifted[SDLK_5] = '%';
	mASCIIshifted[SDLK_6] = '^';
	mASCIIshifted[SDLK_7] = '&';
	mASCIIshifted[SDLK_8] = '*';
	mASCIIshifted[SDLK_9] = '(';
	mASCIIshifted[SDLK_EQUALS] = '+';
	mASCIIshifted[SDLK_QUOTE] = '"';
	mASCIIshifted[SDLK_SLASH] = '?';
	mASCIIshifted[SDLK_COMMA] = '<';
	mASCIIshifted[SDLK_BACKQUOTE] = '~';
	mASCIIshifted[SDLK_LEFTBRACKET] = '{';
	mASCIIshifted[SDLK_RIGHTBRACKET] = '}';
	mASCIIshifted[SDLK_MINUS] = '_';
	mASCIIshifted[SDLK_PERIOD] = '>';
	mASCIIshifted[SDLK_SEMICOLON] = ':';
	mASCIIshifted[SDLK_BACKSLASH] = '|';

	mASCII[SDLK_a] = 'a';
	mASCII[SDLK_b] = 'b';
	mASCII[SDLK_c] = 'c';
	mASCII[SDLK_d] = 'd';
	mASCII[SDLK_e] = 'e';
	mASCII[SDLK_f] = 'f';
	mASCII[SDLK_g] = 'g';
	mASCII[SDLK_h] = 'h';
	mASCII[SDLK_i] = 'i';
	mASCII[SDLK_j] = 'j';
	mASCII[SDLK_k] = 'k';
	mASCII[SDLK_l] = 'l';
	mASCII[SDLK_m] = 'm';
	mASCII[SDLK_n] = 'n';
	mASCII[SDLK_o] = 'o';
	mASCII[SDLK_p] = 'p';
	mASCII[SDLK_q] = 'q';
	mASCII[SDLK_r] = 'r';
	mASCII[SDLK_s] = 's';
	mASCII[SDLK_t] = 't';
	mASCII[SDLK_u] = 'u';
	mASCII[SDLK_v] = 'v';
	mASCII[SDLK_w] = 'w';
	mASCII[SDLK_x] = 'x';
	mASCII[SDLK_y] = 'y';
	mASCII[SDLK_z] = 'z';
	mASCII[SDLK_0] = '0';
	mASCII[SDLK_1] = '1';
	mASCII[SDLK_2] = '2';
	mASCII[SDLK_3] = '3';
	mASCII[SDLK_4] = '4';
	mASCII[SDLK_5] = '5';
	mASCII[SDLK_6] = '6';
	mASCII[SDLK_7] = '7';
	mASCII[SDLK_8] = '8';
	mASCII[SDLK_9] = '9';
	mASCII[SDLK_EQUALS] = '=';
	mASCII[SDLK_QUOTE] = '\'';
	mASCII[SDLK_SLASH] = '/';
	mASCII[SDLK_COMMA] = ',';
	mASCII[SDLK_BACKQUOTE] = '`';
	mASCII[SDLK_LEFTBRACKET] = '[';
	mASCII[SDLK_RIGHTBRACKET] = ']';
	mASCII[SDLK_MINUS] = '-';
	mASCII[SDLK_PERIOD] = '.';
	mASCII[SDLK_SEMICOLON] = ';';
	mASCII[SDLK_BACKSLASH] = '\\';

	mASCII[SDLK_RETURN] = '\n';
	mASCII[SDLK_PAGEUP] = KEY_PUP;
	mASCII[SDLK_PAGEDOWN] = KEY_PDOWN;
	mASCII[SDLK_PAUSE] = KEY_PAUSE;
	mASCII[SDLK_HOME] = KEY_HOME;
	mASCII[SDLK_INSERT] = KEY_INSERT;
	mASCII[SDLK_BACKSPACE] = KEY_BACKSPACE;
	mASCII[SDLK_SPACE] = ' ';
	mASCII[SDLK_ESCAPE] = KEY_ESCAPE;
	mASCII[SDLK_DELETE] = KEY_DELETE;
	mASCII[SDLK_UP] = KEY_UP;
	mASCII[SDLK_DOWN] = KEY_DOWN;
	mASCII[SDLK_LEFT] = KEY_LEFT;
	mASCII[SDLK_RIGHT] = KEY_RIGHT;
	mASCII[SDLK_END] = KEY_END;
	mASCII[SDLK_F1] = KEY_F1;
	mASCII[SDLK_F2] = KEY_F2;
	mASCII[SDLK_F3] = KEY_F3;
	mASCII[SDLK_F4] = KEY_F4;
	mASCII[SDLK_F5] = KEY_F5;
	mASCII[SDLK_F6] = KEY_F6;
	mASCII[SDLK_F7] = KEY_F7;
	mASCII[SDLK_F8] = KEY_F8;
	mASCII[SDLK_F9] = KEY_F9;
	mASCII[SDLK_F10] = KEY_F10;
	mASCII[SDLK_F11] = KEY_F11;
	mASCII[SDLK_F12] = KEY_F12;
}

}

