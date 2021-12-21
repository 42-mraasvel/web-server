#include "utility/utility.hpp"

namespace WebservUtility
{

/*
Return false if not equal
*/
bool caseInsensitiveEqual(const std::string& a, const std::string& b)
{
	if (a.size() != b.size())
	{
		return false;
	}
	
	for (std::size_t i = 0; i < a.size(); ++i)
	{
		if (toupper(a[i]) != toupper(b[i]))
		{
			return false;
		}
	}

	return true;
}

}
