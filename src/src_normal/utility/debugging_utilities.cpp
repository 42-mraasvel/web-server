#include <cstdio>

namespace WebservUtility
{
	void printStrings(char **strings, FILE* stream)
	{
		for (std::size_t i = 0; strings[i] != NULL; ++i)
		{
			fprintf(stream, "%s\n", strings[i]);
		}
	}
}
