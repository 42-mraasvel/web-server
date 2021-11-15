#include "utility.hpp"
#include <string.h>

namespace WebservUtility
{

const char*	basename(const char* x)
{
	const char* y = strrchr(x, '/');

	return y != NULL ? y : x;
}

}
