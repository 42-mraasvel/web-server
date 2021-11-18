#include "settings.hpp"
#include <cstdio>

/*
Wrapper for calling perror and returning ERR code
*/
int syscallError(const std::string& str)
{
	perror(("syscallError: " + str).c_str());
	return ERR;
}

int generalError(const char* format, ...)
{
#ifdef PRINT_ERR
	va_list vp;
	va_start(vp, format);
	vfprintf(stderr, format, vp);
	va_end(vp);
#endif
	return ERR;
}
