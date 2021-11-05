#include "settings.hpp"
#include <stdio.h>

/*
Wrapper for calling perror and returning ERR code
*/
int syscallError(const std::string& str)
{
	perror(("syscallError: " + str).c_str());
	return ERR;
}
