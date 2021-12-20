#include "utility.hpp"

namespace util
{

void convertToLowercase(std::string & s)
{
	for (std::size_t i = 0; i < s.size(); ++ i)
	{
		if (isupper(s[i]))
		{
			s[i] += 32;
		}
	}
}

std::string strToLower(std::string const & s)
{
	std::string x(s);

	for (std::string::iterator it = x.begin(); it != x.end(); ++it)
	{
		*it = tolower(*it);
	}
	return x;
}

}
