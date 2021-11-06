#include "utility/utility.hpp"
#include <fcntl.h>

namespace WebservUtility
{

int makeNonBlocking(int fd)
{
	return fcntl(fd, F_SETFL, O_NONBLOCK);
}

}
