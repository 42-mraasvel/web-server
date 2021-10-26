#include "utility.hpp"

namespace WebservUtility
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

}
