#include "utility/utility.hpp"
#include <sstream>

namespace WebservUtility
{

std::string	itoa(int n)
{
	std::stringstream	ss;
	ss << n;
	return ss.str();
}

}
