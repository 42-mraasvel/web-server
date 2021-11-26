#include "AFdInfo.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/macros.hpp"
#include <unistd.h>
#include <poll.h>

AFdInfo::AFdInfo(): _fd(-1) {}

AFdInfo::AFdInfo(int fd): _fd(fd) {}

AFdInfo::~AFdInfo() {
	closeFd();
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
	return ;
}

void	AFdInfo::closeEvent(FdTable & fd_table)
{
	setToErase();
	fd_table[_index].first.fd = -1;
}

void	AFdInfo::exceptionEvent(FdTable & fd_table)
{
	printf("%sException Event%s: %s\n",
		RED_BOLD, RESET_COLOR, getName().c_str());
}

/* Destruction */

void AFdInfo::setToErase()
{
	closeFd();
	setFlag(AFdInfo::TO_ERASE);
}

void AFdInfo::closeFd()
{
	if (_fd != -1)
	{
		if (close(_fd) == ERR)
		{
			syscallError(_FUNC_ERR("close"));
		}
		printf(BLUE_BOLD "Close Fd:" RESET_COLOR " [%d]\n", _fd);
		_fd = -1;
	}
}

// This function only exists because of FD's not being able to be removed from the FDTable
// See: CGI not in front of the queue not being updated
// Reason: settings the struct pollfd fd to -1 makes poll ignore it
void AFdInfo::closeFd(FdTable & fd_table)
{
	closeFd();
	fd_table[_index].first.fd = -1;
}

AFdInfo::Flags AFdInfo::getFlag() const
{
	return _flag;
}

void AFdInfo::setFlag(AFdInfo::Flags flag)
{
	_flag = flag;
}

