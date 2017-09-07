#include <SDL.h>

#include "Keyboard.h"

#define MXT_KEY_EXTENDED 0xE0
#define MXT_KEY_BREAK 0xF0

namespace z80 {

std::map<uint, uint8_t> Keyboard::mSimple;
std::map<uint, uint8_t> Keyboard::mExtended;

Keyboard::Keyboard(void)
	: intLine_(new bool)
{
    static bool initialized = false;

	intLine_.set(false);

	if(!initialized)
	{
	    init();
		initialized = false;
	}
}

void Keyboard::press(uint key, bool down)
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

void Keyboard::write(uint8_t port, uint8_t data)
{
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
			if(!buf_.empty())
			{
		    	r = buf_.front();
				buf_.pop_front();
			}
			break;
		case 0x0F:
			break;
	}

	return r;
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
}

}

