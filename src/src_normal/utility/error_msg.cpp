#include "settings.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring> // Linux strerror
#include <errno.h>

/*
Wrapper for calling perror and returning ERR code
*/
int syscallError(const std::string& str)
{
	PRINT_ERR << "syscallError: " << str << strerror(errno) << std::endl;
	return ERR;
}

void abortProgram()
{
	exit(1);
}

void abortProgram(std::string const & message)
{
	PRINT_ERR << "Aborting: " << message << std::endl;
	abortProgram();
}
