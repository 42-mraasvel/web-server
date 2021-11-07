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
		flag = AFdInfo::FILE_ERROR;
		return syscallError(_FUNC_ERR("write"));
	}

	_request->message_body.erase(0, n);
	printf("%s: [%d]: Sent: %ld bytes\n",
		getName().c_str(), getFd(), n);
	if (_request->message_body.size() == 0)
	{
		printf("%s: [%d]: Finished writing\n",
			getName().c_str(), getFd());
		updateEvents(WAITING, fd_table);
		flag = AFdInfo::FILE_COMPLETE;
		closeFd();
	}
	return OK;
}

int CgiSender::readEvent(FdTable & fd_table)
{
	std::cerr << "CGI SENDER READ EVENT CALLED: TERMINATING PROGRAM" << std::endl;
	std::terminate();
	return OK;
}

std::string CgiSender::getName() const
{
	return "CgiSender";
}
