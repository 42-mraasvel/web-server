#include "utility.hpp"
#include <string.h>

namespace WebservUtility
{

const char*	ft_basename(const char* x)
{
	const char* y = strrchr(x, '/');

	return y != NULL ? y : x;
}

}
