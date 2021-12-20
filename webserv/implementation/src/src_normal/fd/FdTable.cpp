#include "FdTable.hpp"
#include "settings.hpp"
#include "AFdInfo.hpp"
#include <poll.h>

FdTable::FdTable() {}
FdTable::~FdTable() {}

void	FdTable::insertFd(AFdPointer info)
{
	_pollfd_table.push_back(info->getPollFd());
	_fd_info_table.push_back(info);
	info->setIndex(_pollfd_table.size() - 1);
}

void	FdTable::eraseFd(std::size_t index)
{
/*
1. Assign last element to the to be deleted element
2. Delete the last element
Complexity: O(1)
*/
	_pollfd_table[index] = _pollfd_table.back();
	_pollfd_table.pop_back();
	_fd_info_table.back()->setIndex(index);
	_fd_info_table[index] = _fd_info_table.back();
	_fd_info_table.pop_back();
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
	if (bits & POLLERR)
		str.append("POLLERR ");
	if (bits & POLLNVAL)
		str.append("POLLNVAL ");
	return str;
}

void FdTable::print() const
{
	PRINT_DEBUG << MAGENTA_BOLD "Fd-Table" RESET_COLOR << std::endl;
	for (PollFdTable::const_iterator it = _pollfd_table.begin(); it != _pollfd_table.end(); ++it)
	{
		std::size_t index = (it - _pollfd_table.begin());
		PRINT_DEBUG
		<< "  index: "
		<< index
		<< " | name: " << _fd_info_table[index]->getName()
		<< " | fd: " << it->fd
		<< " | events: "
		<< get_event(it->events)
		<< "| revents: "
		<< get_event(it->revents)
		<< std::endl;
	}
}
