#include "File.hpp"
#include "settings.hpp"
#include "Client.hpp"
#include <unistd.h>
#include <algorithm>
#include <poll.h>
#include <string>
#include <iostream>

#ifdef __linux__
#include <cstring> // for bzero
#endif /* __linux */

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
	char	buffer[BUFFER_SIZE + 1];
	bzero(buffer, BUFFER_SIZE + 1);
	int	ret = read(_fd, buffer, BUFFER_SIZE);
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
	_content.append(std::string(buffer));
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
