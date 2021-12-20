#include "Utility/utility.hpp"
#include <string>
#include <cstring>
#include <limits>

namespace util
{

long strtol(const char* s, int base)
{
	long tmp;
	if (strtol(s, tmp, base) == -1)
	{
		return 0;
	}
	return tmp;
}

long strtol(std::string const & s, int base)
{
	long tmp;
	if (strtol(s, tmp, base) == -1)
	{
		return 0;
	}
	return tmp;
}

int strtol(std::string const & s, long& n, int base)
{
	return strtol(s.c_str(), n, base);
}

bool isBase(char x, int base)
{
	static const std::string base_str = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	for (int i = 0; i < base; ++i)
	{
		if (toupper(x) == base_str[i])
		{
			return true;
		}
	}
	return false;
}

/*
Return: 0 if OK, -1 on ERROR (invalid base or overflow)

Not implemented: octal prefix skip
*/
int strtol(const char* s, long& target, int base)
{

	if (base == 0 || base > 36)
	{
		return -1;
	}

	std::size_t i = 0;
	while (isspace(s[i]))
	{
		++i;
	}
	bool negative = s[i] == '-' ? true : false;
	if (s[i] == '+' || s[i] == '-')
	{
		++i;
	}

	if (base == 16 && std::strncmp(s + i, "0x", 2) == 0)
	{
		i += 2;
	}
	std::numeric_limits<long> limit;
	unsigned long n = 0;
	while (isBase(s[i], base))
	{
		int x = isdigit(s[i]) ? s[i] - '0' : toupper(s[i]) - 'A' + 10;
		n = (n * base) + x;
		// overflow check
		if (n > static_cast<unsigned long>(limit.max())
		&& !(static_cast<long>(n) == limit.min() && negative))
		{
			return -1;
		}
		++i;
	}
	if (negative)
	{
		n = -n;
	}
	target = n;
	return 0;
}

}
