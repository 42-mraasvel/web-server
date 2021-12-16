#include "utility.hpp"
#include "macros.hpp"
#include <unistd.h>
#include <stdio.h>

namespace util
{
	void closeFd(int fd)
	{
		if (close(fd) == -1)
		{
			syscallError(_FUNC_ERR("close"));
		}
	}

	void closePipe(int* fds)
	{
		closeFd(fds[0]);
		closeFd(fds[1]);
	}
}
