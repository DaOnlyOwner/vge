#include "log.h"
#include <iostream>
#include <cassert>

void log::info(const char* msg)
{
	std::cout << "INFO: " << msg;
}

void log::warn(const char* msg)
{
	std::cout << "WARNING: " << msg;
}

void log::fail(const char* msg)
{
	std::cout << "ERROR: " << msg;
	assert(false);
}