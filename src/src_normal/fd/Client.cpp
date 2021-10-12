#include "Client.hpp"
#include "settings.hpp"
#include "fd/File.hpp"
#include <poll.h>
#include <sys/socket.h>

Client::Client(int fd): AFdInfo(fd) {}

struct pollfd	Client::getPollFd() const
{
	struct pollfd temp;
	temp.fd = _fd;
	temp.events = POLLIN | POLLHUP;
	temp.revents = 0;
	return temp;
}


//TODO: Make recv work with multiple iterations, so each iter can loop over request
int	Client::readEvent(FdTable & fd_table)
{
	//TODO: CHECK MAXLEN
	if (_request.size() + BUFFER_SIZE >= _request.capacity())
	{
		_request.reserve(std::max((size_t)BUFFER_SIZE, _request.capacity() * 2));
	}
	int ret = recv(_fd, &_request[_request.size()], BUFFER_SIZE, 0);
	if (ret == ERR)
	{
		perror("Recv");
		return (ERR);
	}
	printf("len read: %d\n",printf("%s\n", _request.c_str()));

	//TODO: Parse Header
	if(_request_parser.parseHeader(_request) == ERR)
	{
		return (ERR);
	}

	if (_executor.execute(this, fd_table, _request_parser) == ERR)
	{
		return ERR;
	}
	resetBuffer();
	updateEvents(WAITING, fd_table); // wait for file to be read

	//TODO:
/*
1. Read request
2. Parser(request)

3. Executor(parser, *this):
	- GET file (open(READING))
	- POST file (open(WRITING))
	- DELETE file


4. Receive signal from executor that resonse is ready
	File:
		if (writtenPost) {
			client->response = x
			client->status = POLLOUT
		} else if (readGet) {
			client->status = POLLOUT;
		}
*/
	return OK;
}

int	Client::writeEvent(FdTable & fd_table)
{
	_executor.generateResponse(_file);

	std::string const & response = _executor.getResponse();

	if (send(_fd, response.c_str(), response.size(), 0) == ERR)
	{
		perror("send");
		return ERR;
	}
	updateEvents(READING, fd_table);
	_file->flag = AFdInfo::TO_ERASE;
	return OK;
}

int	Client::closeEvent()
{
	//TODO:
	return OK;
}

//TODO: retain information about the next request if present
void	Client::resetBuffer()
{
	_request.clear();
}

int	Client::setFile(int file_fd, FdTable & fd_table)
{
	_file = new File(this, file_fd);
	int	file_index = fd_table.size();
	if (fd_table.insertFd(_file) == ERR)
	{
		return ERR;
	}
	_file->updateEvents(AFdInfo::READING, fd_table);

	return OK;
}
