#include "Utility/utility.hpp"
#include <fcntl.h>

namespace util
{

int makeNonBlocking(int fd)
{
	return fcntl(fd, F_SETFL, O_NONBLOCK);
}

}
