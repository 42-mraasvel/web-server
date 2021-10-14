#include "Client.hpp"
#include "settings.hpp"
#include <poll.h>

Client::Client(int fd): AFdInfo(fd)
{
	_handler.setClient(this);
}

struct pollfd	Client::getPollFd() const
{
	struct pollfd temp;
	temp.fd = _fd;
	temp.events = POLLIN;
	temp.revents = 0;
	return temp;
}

int	Client::readEvent(FdTable & fd_table)
{
	if (_handler.processRequest(fd_table) == ERR)
	{
		return ERR;
	}
	return OK;
}

int	Client::writeEvent(FdTable & fd_table)
{
	if (_handler.sendResponse(fd_table) == ERR)
	{
		return ERR;
	}
	return OK;
}

int	Client::closeEvent()
{
	//TODO:
	return OK;
}


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
