#include <string>
#include "utility/utility.hpp"

namespace WebservUtility
{

unsigned long strtoul(std::string const & s)
{
	unsigned long tmp;
	if (strtoul(s, tmp) == -1)
	{
		return 0;
	}
	return tmp;
}

unsigned long strtoul(const char* s)
{
	unsigned long tmp;
	if (strtoul(s, tmp) == -1)
	{
		return 0;
	}
	return tmp;
}

int strtoul(std::string const & s, unsigned long& n)
{
	return strtoul(s.c_str(), n);
}

/*
Return: -1 on OVERFLOW
Stores value inside n
*/
int strtoul(const char* s, unsigned long& n)
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
	n = 0;
	while (isdigit(s[i]))
	{
		// overflow check
		if ((n * 10 + (s[i] - '0')) < n)
		{
			return -1;
		}
		n = (n * 10) + (s[i] - '0');
		++i;
	}
	return 0;
}

}
