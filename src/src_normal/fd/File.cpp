#include "File.hpp"
#include "settings.hpp"
#include "Client.hpp"
#include <unistd.h>
#include <algorithm>
#include <poll.h>
#include <string>
#include <iostream>

File::File(int fd): AFdInfo(fd)
{
	setFlag(AFdInfo::ACTIVE);
}

struct pollfd	File::getPollFd() const
{
	struct pollfd temp;
	temp.fd = _fd;
	temp.events = 0;
	temp.revents = 0;
	return temp;
}

void File::readEvent(FdTable & fd_table)
{
	std::string buffer;
	buffer.resize(BUFFER_SIZE, '\0');
	ssize_t	ret = read(_fd, &buffer[0], BUFFER_SIZE);
	if (ret == ERR)
	{
		perror("read");
		markError(fd_table);
		return;
	}
	if (getFlag() == AFdInfo::ACTIVE)
	{
		setFlag(AFdInfo::START);
	}
	buffer.resize(ret);
	_content.append(buffer);
	if (ret < BUFFER_SIZE) // read EOF
	{
		markFinished(fd_table, AFdInfo::COMPLETE);
	}
}

void File::writeEvent(FdTable & fd_table)
{
	size_t	size = std::min((size_t)BUFFER_SIZE, _content.size());
	if (write(_fd, _content.c_str(), size) == ERR)
	{
		perror("write");
		markError(fd_table);
		return;
	}
	_content.erase(0, size);
	if (_content.empty())
	{
		markFinished(fd_table, AFdInfo::COMPLETE);
	}
}

void File::exceptionEvent(FdTable & fd_table)
{
	AFdInfo::exceptionEvent(fd_table); // RM, REMOVE
	markError(fd_table);
}

std::string const &	File::getContent() const
{
	return _content;
}

void	File::appendContent(std::string & content)
{
	if (content.size() == 0)
	{
		content.swap(_content);
	}
	else
	{
		content.append(_content);
		_content.clear();
	}
}

std::string File::getName() const
{
	return "File";
}

void File::markError(FdTable & fd_table)
{
	_content.clear();
	markFinished(fd_table, AFdInfo::ERROR);
}

void File::markFinished(FdTable & fd_table, AFdInfo::Flags flag)
{
	setFlag(flag);
	updateEvents(AFdInfo::WAITING, fd_table);
}
