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

	struct Character
	{
		static const uint8_t SOLID	 = 0x80;
		static const uint8_t HOLLOW	 = 0x8B;
		static const uint8_t MIX	 = 0x96;
		static const uint8_t END	 = 0xA4;

		static const uint8_t S_LR	 = SOLID + 0;
		static const uint8_t S_UD	 = SOLID + 1;
		static const uint8_t S_LRUD	 = SOLID + 2;
		static const uint8_t S_LU	 = SOLID + 3;
		static const uint8_t S_RU	 = SOLID + 4;
		static const uint8_t S_RD	 = SOLID + 5;
		static const uint8_t S_LD	 = SOLID + 6;
		static const uint8_t S_LRU	 = SOLID + 7;
		static const uint8_t S_LRD	 = SOLID + 8;
		static const uint8_t S_LUD	 = SOLID + 9;
		static const uint8_t S_RUD	 = SOLID + 10;

		static const uint8_t H_LR	 = HOLLOW + 0;
		static const uint8_t H_UD	 = HOLLOW + 1;
		static const uint8_t H_LRUD	 = HOLLOW + 2;
		static const uint8_t H_LU	 = HOLLOW + 3;
		static const uint8_t H_RU	 = HOLLOW + 4;
		static const uint8_t H_RD	 = HOLLOW + 5;
		static const uint8_t H_LD	 = HOLLOW + 6;
		static const uint8_t H_LRU	 = HOLLOW + 7;
		static const uint8_t H_LRD	 = HOLLOW + 8;
		static const uint8_t H_LUD	 = HOLLOW + 9;
		static const uint8_t H_RUD	 = HOLLOW + 10;

		static const uint8_t S_LR_H_U = MIX + 0;
		static const uint8_t S_LR_H_D = MIX + 1;
		static const uint8_t S_UD_H_L = MIX + 2;
		static const uint8_t S_UD_H_R = MIX + 3;
		static const uint8_t S_U_H_LR = MIX + 4;
		static const uint8_t S_D_H_LR = MIX + 5;
		static const uint8_t S_L_H_UD = MIX + 6;
		static const uint8_t S_R_H_UD = MIX + 7;
		static const uint8_t S_LR_H_UD = MIX + 8;
		static const uint8_t S_UD_H_LR = MIX + 9;
		static const uint8_t S_RD_H_LU = MIX + 10;
		static const uint8_t S_LD_H_RU = MIX + 11;
		static const uint8_t S_LU_H_RD = MIX + 12;
		static const uint8_t S_RU_H_LD = MIX + 13;
		static const uint8_t S_ER = END + 0;
		static const uint8_t S_EL = END + 1;
		static const uint8_t S_EU = END + 2;
		static const uint8_t S_ED = END + 3;
		static const uint8_t H_ER = END + 4;
		static const uint8_t H_EL = END + 5;
		static const uint8_t H_EU = END + 6;
		static const uint8_t H_ED = END + 7;
	};
}

#undef MXT_BUFSIZE

#endif

