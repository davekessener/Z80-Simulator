#include <iostream>

#include "Application.h"

#define PROGRAM_FILE "hello.bin"

using namespace z80;

int main(int argc, char *argv[])
try
{
	Application z80sim;

	z80sim.run();

	return 0;
}
catch(const std::string& e)
{
	std::cerr << e << std::endl;

	return 0;
}

