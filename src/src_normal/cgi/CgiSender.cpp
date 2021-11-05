#include "CgiSender.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/macros.hpp"
#include <poll.h>
#include <unistd.h>
#include <algorithm>

CgiSender::CgiSender(int fd, Request* r)
: AFdInfo(fd), _request(r) {}

CgiSender::~CgiSender() {}

struct pollfd CgiSender::getPollFd() const
{
	struct pollfd pfd;

	pfd.fd = _fd;
	// We are only interested in writing to the CGI
	pfd.events = POLLOUT;
	pfd.revents = 0;
	return pfd;
}

int CgiSender::writeEvent(FdTable & fd_table)
{
	ssize_t n = write(_fd, _request->message_body.c_str(),
		std::min<std::size_t>(_request->message_body.size(), BUFFER_SIZE));
	if (n == ERR)
	{
		return syscallError(_FUNC_ERR("write"));
	}

	_request->message_body.erase(0, n);
	printf("Wrote: %ld\n", n);
	if (_request->message_body.size() == 0)
	{
		printf("Finished writing to CGI!\n");
		updateEvents(WAITING, fd_table);
		flag = FILE_COMPLETE;
		if (close(_fd) == ERR) // I think we should close the connection here
		{
			syscallError(_FUNC_ERR("close"));
		}
		_fd = -1;
	}
	exit(0);
	return OK;
}

int CgiSender::readEvent(FdTable & fd_table)
{
	std::cerr << "CGI SENDER READ EVENT CALLED" << std::endl;
	assert(false);
	return OK;
}
