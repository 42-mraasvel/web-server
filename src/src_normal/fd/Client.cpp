#include "Client.hpp"
#include "settings.hpp"
#include <poll.h>

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
	if (_handler.parseRequest(_fd) == ERR)
	{
		return ERR;
	}

	if (_handler.executeMethod(this, fd_table) == ERR)
	{
		return ERR;
	}

	updateEvents(WAITING, fd_table);
	return OK;
}

int	Client::writeEvent(FdTable & fd_table)
{
	if (_handler.sendResponse(_fd) == ERR)
	{
		return ERR;
	}

	updateEvents(READING, fd_table);
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
