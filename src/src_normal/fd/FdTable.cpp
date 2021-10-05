#include "FdTable.hpp"
#include "settings.hpp"
#include "AFdInfo.hpp"
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
	info->setIndex(_pollfd_table.size() - 1);
	return OK;
}

int FdTable::eraseFd(std::size_t index)
{
/*
TODO: call from webserver, which will also close the fd
1. Assign last element to the to be deleted element
2. Delete the last element
Complexity: O(1)
*/
	_pollfd_table[index] = _pollfd_table.back();
	_pollfd_table.pop_back();
	// TODO: close event monitoring
	delete _fd_info_table[index];
	_fd_info_table[index] = _fd_info_table.back();
	_fd_info_table.pop_back();
	_fd_info_table[index]->setIndex(index);
	return OK;
}

FdTable::size_type	FdTable::size()
{
	return _pollfd_table.size();
}

struct pollfd* FdTable::getPointer()
{
	return &_pollfd_table[0];
}

FdTable::pair_t	FdTable::operator[](size_type index)
{
	return pair_t(_pollfd_table[index], _fd_info_table[index]);
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
		std::cout
		<< "  index: "
		<< (it - _pollfd_table.begin())
		<< " | fd: " << it->fd
		<< " | events: "
		<< get_event(it->events)
		<< "| revents: "
		<< get_event(it->revents)
		<< std::endl;
	}
}
