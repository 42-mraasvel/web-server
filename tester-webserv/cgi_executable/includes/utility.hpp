#pragma once

#include <string>

namespace util
{
	unsigned long strtoul(std::string const & s, int base = 10);
	unsigned long strtoul(const char* s, int base = 10);
	int strtoul(std::string const & s, unsigned long& n, int base = 10);
	int strtoul(const char* s, unsigned long& n, int base = 10);
}
