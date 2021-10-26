#include <string>
#include "utility/utility.hpp"

namespace WebservUtility
{

unsigned long strtoul(std::string const & s)
{
	return strtoul(s.c_str());
}

/*
Return: long representation of string, 0 on overflow/underflow
*/
unsigned long strtoul(const char* s)
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
	unsigned long n = 0;
	while (isdigit(s[i]))
	{
		// overflow check
		if ((n * 10 + (s[i] - '0')) < n)
		{
			return 0;
		}
		n = (n * 10) + (s[i] - '0');
		++i;
	}
	return n;
}

}
