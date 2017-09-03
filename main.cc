#include <iostream>

#include "Application.h"

#define PROGRAM_FILE "hello.bin"

using namespace z80;

int main(int argc, char *argv[])
{
	Application z80sim;

	z80sim.run();

	return 0;
}

