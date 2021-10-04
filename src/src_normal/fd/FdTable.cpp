#include "FdTable.hpp"
#include "settings.hpp"
#include <poll.h>

int	FdTable::insertFd(AFdInfo *info)
{
	this->_pollfd_table.push_back(info->getPollFd());
	this->_fd_table.push_back(info);
	return OK;
}

int	FdTable::eraseFd(int fd)
{
	// write erase function with iterator bullshit blablabla
	return OK;
}








