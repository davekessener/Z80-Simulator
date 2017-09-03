#include <fstream>
#include <iterator>
#include <vector>
#include <algorithm>

#include "Program.h"

namespace z80 {

Program::Program(const std::string& fn)
{
	std::ifstream in(fn, std::ios::binary);
	std::vector<char> buf(
		(std::istreambuf_iterator<char>(in)), 
		(std::istreambuf_iterator<char>()));

	data_ = new uint8_t[len_ = buf.size()];

	std::copy(buf.cbegin(), buf.cend(), data_);
}

Program::~Program(void)
{
	delete[] data_;
}

}

