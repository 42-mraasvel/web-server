#include "Client.hpp"
#include "settings.hpp"
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
	// updateEvents(WRITING, fd_table);
	return OK;
}

int	Client::writeEvent(FdTable & fd_table)
{
	//TODO:
	std::string str(BUFFER_SIZE, 'a');
	ssize_t n = send(_fd, str.c_str(), str.size(), 0);
	if (n == ERR)
	{
		perror("send");
	}
	printf("Send: %ld of %d bytes\n", n, BUFFER_SIZE);
	updateEvents(READING, fd_table);
	return OK;
}

int	Client::closeEvent()
{
	//TODO:
	return OK;
}

void	Client::updateEvents(Client::EventTypes type, FdTable & fd_table)
{
	short int updated_events;

	switch (type)
	{
		case Client::READING:
			updated_events = POLLIN;
			break;
		case Client::WRITING:
			updated_events = POLLOUT;
			break;
		case Client::WAITING:
			updated_events = 0;
			break;
	}
	fd_table[_index].first.events = updated_events | POLLHUP;
}
