#include <vector>
#include <string>
#include "utility.hpp"
#include <iostream> // RM

namespace WebservUtility
{

/*
LOGIC:
	1. SKIP DELIM
	2. FIND next DELIM (or npos)
	3. SUBSTR until that DELIM
*/

/*
create vector containing substrings from input split by delim
*/
std::vector<std::string> splitString(const std::string& input, char delim)
{
	std::string delim_set;
	delim_set.push_back(delim);
	return WebservUtility::splitString(input, delim_set);
}

std::vector<std::string> splitString(const std::string& input, const std::string& delim_set)
{
	std::vector<std::string> strings;

	std::size_t i = input.find_first_not_of(delim_set);
	while (i < input.size())
	{
		std::size_t end = input.find_first_of(delim_set, i);
		strings.push_back(input.substr(i, end - i));
		i = input.find_first_not_of(delim_set, end);
	}
	return strings;
}

}
