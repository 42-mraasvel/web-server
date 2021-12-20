#include <unistd.h>

#ifndef __linux__
extern char** environ;
#endif

namespace util
{

/* Return the global environment variable */
char** getEnvp()
{
	return environ;
}

}
