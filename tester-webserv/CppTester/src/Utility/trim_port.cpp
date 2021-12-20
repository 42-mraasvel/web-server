#include "utility.hpp"
#include <string>

namespace util
{
	/*
		'localhost':8080 -> 'localhost'
	*/
	std::string trimPort(std::string const & str)
	{
		std::size_t n = str.rfind(":");
		if (n != std::string::npos)
		{
			return str.substr(0, n);
		}
		return str;
	}
}
