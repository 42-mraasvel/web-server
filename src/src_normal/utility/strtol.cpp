#include "utility/utility.hpp"
#include <string>
#include <limits>

namespace WebservUtility
{

long strtol(const char* s)
{
	long tmp;
	if (strtol(s, tmp) == -1)
	{
		return 0;
	}
	return tmp;
}

long strtol(std::string const & s)
{
	long tmp;
	if (strtol(s, tmp) == -1)
	{
		return 0;
	}
	return tmp;
}

int strtol(std::string const & s, long& n)
{
	return strtol(s.c_str(), n);
}

/*
Return: 0 if OK, -1 on OVERFLOW
*/
int strtol(const char* s, long& target)
{
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
	std::numeric_limits<long> limit;
	unsigned long n = 0;
	while (isdigit(s[i]))
	{
		n = (n * 10) + (s[i] - '0');
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
