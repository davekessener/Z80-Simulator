#include <regex>
#include <fstream>
#include <iostream>
#include <unistd.h>

#include "Assembler.h"

using namespace z80;

void test( );

int main(int argc, char *argv[])
try
{
	if(argc < 3)
	{
		std::cerr << "ERR: " << argv[0] << " src.s dst.bin" << std::endl;

		return 1;
	}
	else
	{
		Assembler a(argv[1]);

		auto p = a.get();

		if(!p.errors.empty())
		{
			for(const auto& e : p.errors)
			{
				std::cerr << "ERR [" << e.meta.file << ":" << e.meta.line + 1 << "] " << e.message << std::endl;
				std::cerr << ">>> \"" << e.meta.source << "\"" << std::endl;
			}

			return 1;
		}
		else
		{
			for(const auto& e : p.data)
			{
				std::string data;
				for(const auto& v : e.data)
				{
					char b[10];
					snprintf(b, 10, "%02X ", (uint)v);
					data += std::string((const char *) b);
				}
				printf("$%04X: %-12s;; % 3d %s\n", e.address, data.c_str(), e.meta.line, e.meta.source.c_str());
			}

			std::ofstream out(argv[2], std::ios::out | std::ios::binary);

			if(!out.good())
			{
				std::cerr << "ERR: failed to open output file \"" << argv[2] << "\"!" << std::endl;

				return 1;
			}

			uint size = 0;

			for(const auto& e : p.data)
			{
				for(const auto& v : e.data)
				{
					out.write((const char *)(&v), 1); ++size;
				}
			}

			out.close();

			std::cout << "Wrote " << size << " bytes to file " << argv[2] << "." << std::endl;

			return 0;
		}
	}
}
catch(const std::string& e)
{
	std::cout << "Critical error: " << e << std::endl;

	return 0;
}

