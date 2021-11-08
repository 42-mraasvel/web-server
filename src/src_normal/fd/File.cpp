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
	flag = AFdInfo::ACTIVE;
}

struct pollfd	File::getPollFd() const
{
	struct pollfd temp;
	temp.fd = _fd;
	temp.events = 0;
	temp.revents = 0;
	return temp;
}

int File::readEvent(FdTable & fd_table)
{
	std::string buffer;
	buffer.resize(BUFFER_SIZE, '\0');
	ssize_t	ret = read(_fd, &buffer[0], BUFFER_SIZE);
	if (ret == ERR)
	{
		perror("read");
		this->updateEvents(AFdInfo::WAITING, fd_table);
		flag = AFdInfo::FILE_ERROR;
		return ERR;
	}
	if (flag == AFdInfo::ACTIVE)
	{
		flag = AFdInfo::FILE_START;
	}
	buffer.resize(ret);
	_content.append(buffer);
	if (ret < BUFFER_SIZE) // read EOF
	{
		this->updateEvents(AFdInfo::WAITING, fd_table);
		flag = AFdInfo::FILE_COMPLETE;
	}
	return OK;
}

int File::writeEvent(FdTable & fd_table)
{
	size_t	size = std::min((size_t)BUFFER_SIZE, _content.size());
	if (write(_fd, _content.c_str(), size) == ERR)
	{
		perror("write");
		this->updateEvents(AFdInfo::WAITING, fd_table);
		flag = AFdInfo::FILE_ERROR;
		return ERR;
	}
	_content.erase(0, size);
	if (_content.empty())
	{
		this->updateEvents(AFdInfo::WAITING, fd_table);
		flag = AFdInfo::FILE_COMPLETE;
	}
	return OK;
}

std::string const &	File::getContent() const
{
	return _content;
}

void	File::setContent(std::string const & content)
{
	_content = content;
}

void	File::clearContent()
{
	_content.clear();
}

void	File::swapContent(std::string & content)
{
	_content.swap(content);
}
