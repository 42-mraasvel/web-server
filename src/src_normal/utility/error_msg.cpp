#include "settings.hpp"
#include <cstdio>
#include <cstdlib>

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
	fprintf(stderr, RED_BOLD "ERROR: " RESET_COLOR);
	vfprintf(stderr, format, vp);
	va_end(vp);
#endif
	return ERR;
}

void abortProgram()
{
	std::abort();
}

void abortProgram(std::string const & message)
{
	std::cerr << RED_BOLD "Aborting: " << message << RESET_COLOR << std::endl;
	abortProgram();
}
