#ifndef LIB_WINUI_H
#define LIB_WINUI_H

#include <algorithm>
#include <cstdint>

using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

typedef unsigned uint;

namespace winui
{
	struct Position
	{
		Position( ) : x(0), y(0) { }
		Position(int x_, int y_) : x(x_), y(y_) { }

		int x, y;
	};

	struct Dimension
	{
		Dimension( ) : w(0), h(0) { }
		Dimension(uint w_, uint h_) : w(w_), h(h_) { }

		uint w, h;
	};

	struct Space
	{
		Space( ) : pos(0, 0), dim(0, 0) { }
		Space(const Dimension& d) : pos(0, 0), dim(d) { }
		Space(const Position& p, const Dimension& d) : pos(p), dim(d) { }
		Space(uint w, uint h) : pos(0, 0), dim(w, h) { }
		Space(int x, int y, uint w, uint h) : pos(x, y), dim(w, h) { }

		Space& operator+=(const Space& s)
		{
			pos.x += s.pos.x;
			pos.y += s.pos.y;
			dim.w  = std::min(dim.w - s.pos.x, s.dim.w);
			dim.h  = std::min(dim.h - s.pos.y, s.dim.h);

			return *this;
		}

		Position pos;
		Dimension dim;
	};

	struct Color
	{
		Color( ) : r(0), g(0), b(0) { }
		Color(uint8_t r_, uint8_t g_, uint8_t b_) : r(r_), g(g_), b(b_) { }

		static Color BLACK()  { return Color(0x00, 0x00, 0x00); }
		static Color WHITE()  { return Color(0xFF, 0xFF, 0xFF); }
		static Color RED()    { return Color(0xFF, 0x00, 0x00); }
		static Color GREEN()  { return Color(0x00, 0xFF, 0x00); }
		static Color BLUE()   { return Color(0x00, 0x00, 0xFF); }
		static Color YELLOW() { return Color(0xFF, 0xFF, 0x00); }
		static Color MAGENTA(){ return Color(0xFF, 0x00, 0xFF); }
		static Color CYAN()   { return Color(0x00, 0xFF, 0xFF); }

		uint8_t r, g, b;
	};
}

#endif

