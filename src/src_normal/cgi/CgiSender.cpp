#include "CgiSender.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/macros.hpp"
#include "utility/status_codes.hpp"
#include "utility/Timer.hpp"
#include <poll.h>
#include <unistd.h>
#include <algorithm>

CgiSender::CgiSender(int fd, Request* r, Timer* timer)
: AFdInfo(fd), _timer(timer) {
	//TODO: determine location and clean solution to this
	if (r->method == Method::POST)
	{
		_message_body.swap(r->message_body);
	}
	else
	{
		r->message_body.clear();
	}
}

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
	_timer->reset();

	std::size_t len = std::min<std::size_t>(_message_body.size(), BUFFER_SIZE);
	if (len != 0)
	{
		ssize_t n = write(_fd, _message_body.c_str(), len);
		if (n == ERR)
		{
			closeEvent(fd_table, AFdInfo::ERROR, StatusCode::INTERNAL_SERVER_ERROR);
			return syscallError(_FUNC_ERR("write"));
		}
		_message_body.erase(0, n);
		printf("%s: [%d]: Sent: %ld bytes\n",
			getName().c_str(), getFd(), n);
	}

	if (_message_body.size() == 0)
	{
		printf("%s: [%d]: Finished writing\n",
			getName().c_str(), getFd());
		closeEvent(fd_table);
	}
	return OK;
}

int CgiSender::readEvent(FdTable & fd_table)
{
	std::cerr << "CGI SENDER READ EVENT CALLED: TERMINATING PROGRAM" << std::endl;
	std::terminate();
	return OK;
}

void CgiSender::closeEvent(FdTable & fd_table)
{
	_timer->reset();
	if (_message_body.size() == 0)
	{
		closeEvent(fd_table, AFdInfo::COMPLETE, StatusCode::STATUS_OK);
	}
	else
	{
		closeEvent(fd_table, AFdInfo::ERROR, StatusCode::BAD_GATEWAY);
	}
}

void CgiSender::closeEvent(FdTable & fd_table, AFdInfo::Flags flag, int status_code)
{
	setFlag(flag);
	updateEvents(AFdInfo::WAITING, fd_table);
	_status_code = status_code;
	closeFd(fd_table);
}

std::string CgiSender::getName() const
{
	return "CgiSender";
}
