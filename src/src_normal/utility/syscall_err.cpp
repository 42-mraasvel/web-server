#include "settings.hpp"
#include <stdio.h>

/*
Wrapper for calling perror and returning ERR code
*/
int syscallError(const char* str)
{
	perror(str);
	return ERR;
}
