#include "CgiReader.hpp"
#include "settings.hpp"
#include "utility/macros.hpp"
#include "utility/utility.hpp"
#include "utility/Timer.hpp"
#include <poll.h>
#include <cassert>
#include <unistd.h>
#include <cstdlib>

CgiReader::CgiReader(int fd, Timer* timer)
: AFdInfo(fd), _timer(timer) {}

CgiReader::~CgiReader() {}

struct pollfd CgiReader::getPollFd() const
{
	struct pollfd pfd;

	pfd.fd = _fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	return pfd;
}

void CgiReader::writeEvent(FdTable & fd_table)
{
	abortProgram("[" RED_BOLD "ERROR" RESET_COLOR "] "
		"CgiReader::writeEvent() called: "
		RED_BOLD "ABORING PROGRAM" RESET_COLOR "\n");
}

void CgiReader::readEvent(FdTable & fd_table)
{
	_timer->reset();

	std::string buffer(BUFFER_SIZE + 1, '\0');

	ssize_t n = read(_fd, &buffer[0], BUFFER_SIZE);
	if (n == ERR)
	{
		syscallError(_FUNC_ERR("read"));
		closeEvent(fd_table, AFdInfo::ERROR, StatusCode::INTERNAL_SERVER_ERROR);
		return;
	}
	else if (n == 0)
	{
		closeEvent(fd_table);
		return;
	}

	buffer.resize(n);
	parseBuffer(fd_table, buffer);
	std::cout << buffer << std::endl;
}

void CgiReader::parseBuffer(FdTable & fd_table, std::string const & buffer)
{
	_parser.parse(buffer);

	if (_parser.isError())
	{
		printf("Reader Parsing ERROR\n");
		closeEvent(fd_table, AFdInfo::ERROR, _parser.getStatusCode());
	}
	else if (_parser.isComplete())
	{
		closeEvent(fd_table, AFdInfo::COMPLETE);
	}
}

void CgiReader::closeEvent(FdTable & fd_table)
{
	_timer->reset();
	if (!_parser.isComplete() && !_parser.isCompleteIfEof())
	{
		return closeEvent(fd_table, AFdInfo::ERROR, StatusCode::BAD_GATEWAY);
	}
	closeEvent(fd_table, AFdInfo::COMPLETE);
}

void CgiReader::closeEvent(FdTable & fd_table, AFdInfo::Flags flag)
{
	closeEvent(fd_table, flag, StatusCode::STATUS_OK);
}

void CgiReader::closeEvent(FdTable & fd_table, AFdInfo::Flags flag, int status_code)
{
	setFlag(flag);
	updateEvents(AFdInfo::WAITING, fd_table);
	_status_code = status_code;
	closeFd(fd_table);
}

void CgiReader::exceptionEvent(FdTable & fd_table)
{
	AFdInfo::exceptionEvent(fd_table); // RM, REMOVE
	clear();
	closeEvent(fd_table, AFdInfo::ERROR, StatusCode::INTERNAL_SERVER_ERROR);
}

void CgiReader::clear()
{
	_parser.reset();
}

/* Interfacing Functions */

int CgiReader::getStatusCode() const
{
	return _status_code;
}

HeaderField & CgiReader::getHeader()
{
	return _parser.getHeader();
}

std::string & CgiReader::getBody()
{
	return _parser.getContent();
}

std::string CgiReader::getName() const
{
	return "CgiReader";
}
