#include "utility/utility.hpp"
#include <string>
#include <limits>

namespace WebservUtility
{

long strtol(std::string const & s)
{
	return strtol(s.c_str());
}

/*
Return: long representation of string, 0 on overflow/underflow
*/
long strtol(const char* s)
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
			return 0;
		}
		++i;
	}
	if (negative)
	{
		return -n;
	}
	return n;
}

}
