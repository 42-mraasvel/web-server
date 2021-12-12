#include "utility.hpp"
#include <string.h>

namespace util
{

const char*	ft_basename(const char* x)
{
	const char* y = strrchr(x, '/');

	return y != NULL ? y : x;
}

}
