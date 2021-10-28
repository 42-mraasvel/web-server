#include "AFdInfo.hpp"
#include "settings.hpp"
#include <unistd.h>
#include <poll.h>

AFdInfo::AFdInfo(): _fd(-1) {}

AFdInfo::AFdInfo(int fd): _fd(fd) {}

AFdInfo::~AFdInfo()
{
	if (close(_fd) == ERR)
	{
		perror("close");
	}
}

int	AFdInfo::getFd() const
{
	return this->_fd;
}

std::size_t AFdInfo::getIndex() const
{
	return _index;
}

void AFdInfo::setIndex(std::size_t index)
{
	_index = index;
}

void	AFdInfo::updateEvents(AFdInfo::EventTypes type, FdTable & fd_table)
{
	short int updated_events;

	switch (type)
	{
		case AFdInfo::READING:
			updated_events = POLLIN;
			break;
		case AFdInfo::WRITING:
			updated_events = POLLOUT;
			break;
		case AFdInfo::WAITING:
			updated_events = 0;
			break;
	}
	fd_table[_index].first.events = updated_events;
}

void	AFdInfo::update(FdTable & fd_table)
{
	if (flag == AFdInfo::TO_ERASE)
	{
		printf(BLUE_BOLD "Close File:" RESET_COLOR " [%d]\n", _fd);
		fd_table.eraseFd(_index);
	}
}
