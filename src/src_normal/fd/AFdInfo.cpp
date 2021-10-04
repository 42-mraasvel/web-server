#include "AFdInfo.hpp"

AFdInfo::AFdInfo(): _fd(-1) {}

AFdInfo::AFdInfo(int fd): _fd(fd) {}

AFdInfo::~AFdInfo() {}

int	AFdInfo::getFd() const
{
	return this->_fd;
}

