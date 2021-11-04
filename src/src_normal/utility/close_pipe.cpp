#include <unistd.h>
#include <stdio.h>

namespace WebservUtility
{

	namespace __detail__
	{
		void closeFdError(int fd)
		{
			if (close(fd) == -1)
			{
				perror("close");
			}
		}
	}

	void closePipe(int* fds)
	{
		__detail__::closeFdError(fds[0]);
		__detail__::closeFdError(fds[1]);
	}
}
