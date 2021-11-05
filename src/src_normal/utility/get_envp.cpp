#include <unistd.h>

namespace WebservUtility
{

/* Return the global environment variable */
char** getEnvp()
{
	#ifdef __linux__
		return environ;
	#else
		extern char** __environ;
		return __environ;
	#endif
}

}
