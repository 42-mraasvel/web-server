#include "FdTable.hpp"
#include "settings.hpp"
#include <poll.h>

FdTable::~FdTable()
{
	for (FdVector_t::iterator it = _fd_table.begin(); it != _fd_table.end(); ++it)
	{
		delete *it;
	}
}

int	FdTable::insertFd(AFdInfo *info)
{
	this->_pollfd_table.push_back(info->getPollFd());
	this->_fd_table.push_back(info);
	return OK;
}

int	FdTable::eraseFd(int fd)
{
	PollFdTable::iterator it = findFd(fd);
	if (it == _pollfd_table.end())
	{
		return OK;
	}
/*
Called by the closeEvent() function
1. Assign last element to the to be deleted element
2. Delete the last element
Complexity: O(1)
*/
	std::size_t index = it - _pollfd_table.begin();
	_pollfd_table[index] = _pollfd_table.back();
	_pollfd_table.pop_back();
	delete _fd_table[index];
	_fd_table[index] = _fd_table.back();
	_fd_table.pop_back();
	return OK;
}

FdTable::PollFdTable::iterator FdTable::findFd(int fd)
{
	for (PollFdTable::iterator it = _pollfd_table.begin(); it != _pollfd_table.end(); ++it)
	{
		if (it->fd == fd)
		{
			return it;
		}
	}
	return this->_pollfd_table.end();
}


