#include "AFdInfo.hpp"
#include "settings.hpp"
#include "utility/Output.hpp"
#include "utility/macros.hpp"
#include <unistd.h>
#include <poll.h>

AFdInfo::AFdInfo(): _flag(ACTIVE), _fd(-1) {}

AFdInfo::AFdInfo(int fd) : _flag(ACTIVE), _fd(fd) {}

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

static short int	convertEventType(AFdInfo::EventTypes type)
{
	switch (type)
	{
		case AFdInfo::READING:
			return POLLIN;
		case AFdInfo::WRITING:
			return POLLOUT;
		case AFdInfo::WAITING:
			return 0;
	}
}

void	AFdInfo::updateEvents(AFdInfo::EventTypes type, FdTable & fd_table)
{
	fd_table[_index].first.events = convertEventType(type);
}

void	AFdInfo::addEvents(AFdInfo::EventTypes type, FdTable & fd_table)
{
	fd_table[_index].first.events |= convertEventType(type);
}

void	AFdInfo::removeEvents(AFdInfo::EventTypes type, FdTable & fd_table)
{
	fd_table[_index].first.events &= ~(convertEventType(type));
}

void	AFdInfo::update(FdTable & fd_table)
{
	(void)fd_table;
	return ;
}

void	AFdInfo::closeEvent(FdTable & fd_table)
{
	setToErase();
	fd_table[_index].first.fd = -1;
}

void	AFdInfo::exceptionEvent(FdTable & fd_table)
{
	(void)fd_table;
	PRINT_WARNING << "Exception Event: " << getName() << std::endl;
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
		PRINT_DEBUG << BLUE_BOLD "Close Fd" RESET_COLOR ": [" << _fd << "]" << std::endl;
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
