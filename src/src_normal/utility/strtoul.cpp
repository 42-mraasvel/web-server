#include <string>
#include <cstring>
#include "utility/utility.hpp"

namespace WebservUtility
{

unsigned long strtoul(std::string const & s, int base)
{
	unsigned long tmp;
	if (strtoul(s, tmp, base) == -1)
	{
		return 0;
	}
	return tmp;
}

unsigned long strtoul(const char* s, int base)
{
	unsigned long tmp;
	if (strtoul(s, tmp, base) == -1)
	{
		return 0;
	}
	return tmp;
}

int strtoul(std::string const & s, unsigned long& n, int base)
{
	return strtoul(s.c_str(), n, base);
}

/*
Return: -1 on OVERFLOW
Stores value inside n
*/
int strtoul(const char* s, unsigned long& n, int base)
{
	std::size_t i = 0;
	while (isspace(s[i]))
	{
		++i;
	}
	if (s[i] == '+')
	{
		++i;
	}

	if (base == 16 && std::strncmp(s + i, "0x", 2) == 0)
	{
		i += 2;
	}
	n = 0;
	while (isBase(s[i], base))
	{
		// overflow check
		if ((n * base + (s[i] - '0')) < n)
		{
			return -1;
		}
		if (isdigit(s[i]))
		{
			n = (n * base) + (s[i] - '0');
		}
		else
		{
			n = (n * base) + (toupper(s[i]) - 'A' + 10);
		}
		++i;
	}
	return 0;
}

}
