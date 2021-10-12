#include <string>
#include <iostream>

namespace WebservUtility
{

/*
Searches at most LIMIT characters for the substr to_find

s -- string to search
to_find -- substr to search for
limit -- max characters to iterate over
*/
std::size_t findLimit(std::string const & s, std::string const & to_find, std::size_t limit)
{
	if (limit < to_find.size() || to_find.size() > s.size())
	{
		return std::string::npos;
	}

	std::size_t i = 0;
	while (i <= s.size() - to_find.size() && i <= limit - to_find.size())
	{
		if (s.compare(i, to_find.size(), to_find) == 0)
		{
			return i;
		}
		++i;
	}

	return std::string::npos;
}

}
