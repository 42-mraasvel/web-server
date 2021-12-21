#include "utility.hpp"
#include "macros.hpp"
#include <unistd.h>

namespace WebservUtility
{

	namespace __detail__
	{
		void closeFdError(int fd)
		{
			if (close(fd) == -1)
			{
				syscallError(_FUNC_ERR("close"));
			}
		}
	}

	void closePipe(int* fds)
	{
		__detail__::closeFdError(fds[0]);
		__detail__::closeFdError(fds[1]);
	}
}
