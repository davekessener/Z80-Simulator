#ifndef Z80_PROGRAM_H
#define Z80_PROGRAM_H

#include <string>
#include <cstdint>

namespace z80
{
	class Program
	{
		public:
			Program(const std::string&);
			~Program( );
			const uint8_t *data( ) const { return data_; }
			size_t length( ) const { return len_; }
		private:
			uint8_t *data_;
			size_t len_;
	};
}

#endif

