#include "FdTable.hpp"
#include "settings.hpp"
#include <poll.h>

FdTable::~FdTable()
{
	for (FdVector_t::iterator it = _fd_info_table.begin(); it != _fd_info_table.end(); ++it)
	{
		delete *it;
	}
}

int	FdTable::insertFd(AFdInfo *info)
{
	_pollfd_table.push_back(info->getPollFd());
	_fd_info_table.push_back(info);
	return OK;
}

int FdTable::eraseFd(PollFdTable::iterator it)
{
/*
TODO: call from webserver, which will also close the fd
1. Assign last element to the to be deleted element
2. Delete the last element
Complexity: O(1)
*/
	std::size_t index = it - _pollfd_table.begin();
	_pollfd_table[index] = _pollfd_table.back();
	_pollfd_table.pop_back();
	delete _fd_info_table[index];
	_fd_info_table[index] = _fd_info_table.back();
	_fd_info_table.pop_back();
	return OK;
}

int	FdTable::eraseFd(int fd)
{
	PollFdTable::iterator it = findFd(fd);
	if (it == _pollfd_table.end())
	{
		return ERR;
	}
	eraseFd(it);
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

/* Debugging */

static std::string	get_event(short bits)
{
	std::string str("");
	
	if (bits & POLLIN)
		str.append("POLLIN ");
	if (bits & POLLOUT)
		str.append("POLLOUT ");
	if (bits & POLLHUP)
		str.append("POLLHUP ");
	return str;
}

void FdTable::print() const
{
	std::cout << MAGENTA_BOLD "Fd-Table" RESET_COLOR << std::endl;
	for (PollFdTable::const_iterator it = _pollfd_table.begin(); it != _pollfd_table.end(); ++it)
	{
		std::cout << "  fd: " << it->fd << " | events: " << get_event(it->events) << "| revents: " << get_event(it->revents) << std::endl;
	}
}
