#include "CgiSender.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/macros.hpp"
#include "utility/status_codes.hpp"
#include "utility/Timer.hpp"
#include <poll.h>
#include <unistd.h>
#include <algorithm>

CgiSender::CgiSender(int fd, SmartPointer<Request> r, Timer* timer)
: AFdInfo(fd), _timer(timer) {
	//TODO: DISCUSS: message body is only sent in a POST request to the CGI process
	_message_body = r->message_body;
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

void CgiSender::writeEvent(FdTable & fd_table)
{
	_timer->reset();

	std::size_t len = std::min<std::size_t>(_message_body.size(), BUFFER_SIZE);
	if (len != 0)
	{
		ssize_t n = write(_fd, _message_body.c_str(), len);
		if (n == ERR)
		{
			closeEvent(fd_table, AFdInfo::ERROR, StatusCode::INTERNAL_SERVER_ERROR);
			syscallError(_FUNC_ERR("write"));
			return;
		}
		_message_body.erase(0, n);
		// printf("%s: [%d]: Sent: %ld bytes\n",
		// 	getName().c_str(), getFd(), n);
	}

	if (_message_body.size() == 0)
	{
		// printf("%s: [%d]: Finished writing\n",
		// 	getName().c_str(), getFd());
		closeEvent(fd_table);
	}
}

void CgiSender::readEvent(FdTable & fd_table)
{
	abortProgram(RED_BOLD "CGI SENDER READ EVENT CALLED: ABORTING PROGRAM" RESET_COLOR);
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
	clear();
	setFlag(flag);
	updateEvents(AFdInfo::WAITING, fd_table);
	_status_code = status_code;
	closeFd(fd_table);
}

void CgiSender::clear()
{
	_message_body.clear();
}

void CgiSender::exceptionEvent(FdTable & fd_table)
{
	AFdInfo::exceptionEvent(fd_table); // RM, REMOVE
	closeEvent(fd_table, AFdInfo::ERROR, StatusCode::INTERNAL_SERVER_ERROR);
}

int CgiSender::getStatusCode() const
{
	return _status_code;
}

std::string CgiSender::getName() const
{
	return "CgiSender";
}
