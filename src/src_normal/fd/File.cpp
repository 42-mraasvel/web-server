#include "File.hpp"
#include "settings.hpp"
#include "Client.hpp"
#include <unistd.h>
#include <algorithm>
#include <poll.h>
#include <string>

File::File(int fd): AFdInfo(fd) {}

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
	//TODO: keep track of total size read -> content-length
	char	buf[BUFFER_SIZE];
	int	ret = read(_fd, buf, BUFFER_SIZE);
	if (ret == ERR)
	{
		perror("read in File::readEvent()");
		return ERR;
	}
	_content.append(std::string(buf));
	if (ret < BUFFER_SIZE) // read EOF
	{
		this->updateEvents(AFdInfo::WAITING, fd_table);
		flag = AFdInfo::EVENT_COMPLETE;
	}
	return OK;
}

int File::writeEvent(FdTable & fd_table)
{
	size_t	size = std::min((size_t)BUFFER_SIZE, _content.size());
	if (write(_fd, _content.c_str(), size) == ERR)
	{
		perror("write");
		return ERR;
	}
	_content.erase(0, size);
	if (size < BUFFER_SIZE)
	{
		this->updateEvents(AFdInfo::WAITING, fd_table);
		flag = AFdInfo::EVENT_COMPLETE;
	}
	return OK;
}

int	File::closeEvent()
{
	return close(_fd);
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

void	File::appendContent(std::string const & content)
{
	_content.append(content);
}
