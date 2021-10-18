#include "File.hpp"
#include "settings.hpp"
#include "Client.hpp"
#include <unistd.h>
#include <poll.h>
#include <string>

File::File(int client_index, int fd): AFdInfo(fd), _client_index(client_index) {}

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
	//TODO: to discuss with team how to read directly into _content;
	//TODO: iterative reading to combine with Client::readEvent().

	char	buf[BUFFER_SIZE];
	int	ret = read(_fd, buf, BUFFER_SIZE);
	if (ret == ERR)
	{
		perror("read");
		return ERR;
	}
	_content = std::string(buf);

	fd_table[_client_index].second->updateEvents(AFdInfo::WRITING, fd_table);
	this->updateEvents(AFdInfo::WAITING, fd_table);

	return OK;
}

int File::writeEvent(FdTable & fd_table)
{
	int	ret = write(_fd, _content.c_str(), _content.size());
	if (ret == ERR)
	{
		perror("write");
		return ERR;
	}

	fd_table[_client_index].second->updateEvents(AFdInfo::WRITING, fd_table);
	this->updateEvents(AFdInfo::WAITING, fd_table);

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
