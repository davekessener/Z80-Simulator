#ifndef Z80_LIB_H
#define Z80_LIB_H

#include <string>
#include <cstdint>
#include <cstdarg>

#define MXT_BUFSIZE 1024

typedef unsigned uint;

namespace lib
{
	inline std::string stringf(const std::string& s, ...)
	{
		char buf[MXT_BUFSIZE];
		va_list args;

		va_start(args, s);

		vsnprintf(buf, MXT_BUFSIZE, s.c_str(), args);

		va_end(args);

		buf[MXT_BUFSIZE-1] = '\0';

		return std::string(buf);
	}
}

#undef MXT_BUFSIZE

#endif

