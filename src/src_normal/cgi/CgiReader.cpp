#include "CgiReader.hpp"
#include "settings.hpp"
#include "utility/macros.hpp"
#include "utility/utility.hpp"
#include <poll.h>
#include <cassert>
#include <unistd.h>

CgiReader::CgiReader(int fd)
: AFdInfo(fd) {}

CgiReader::~CgiReader() {}

struct pollfd CgiReader::getPollFd() const
{
	struct pollfd pfd;

	pfd.fd = _fd;
	pfd.events = POLLIN;
	pfd.revents = 0;
	return pfd;
}

int	CgiReader::writeEvent(FdTable & fd_table)
{
	fprintf(stderr, "[" RED_BOLD "ERROR" RESET_COLOR "] "
		"CgiReader::writeEvent() called: "
		RED_BOLD "TERMINATING PROGRAM" RESET_COLOR "\n");
	std::terminate();
	return OK;
}

int	CgiReader::readEvent(FdTable & fd_table)
{
	std::string buffer(BUFFER_SIZE + 1, '\0');

	ssize_t n = read(_fd, &buffer[0], BUFFER_SIZE);
	if (n == ERR)
	{
		syscallError(_FUNC_ERR("read"));
		closeEvent(fd_table, AFdInfo::FILE_ERROR);
		return ERR;
	}
	else if (n == 0)
	{
		closeEvent(fd_table, AFdInfo::FILE_COMPLETE);
		return OK;
	}

	buffer.resize(n);
	parseBuffer(fd_table, buffer);
	return OK;
}

void CgiReader::parseBuffer(FdTable & fd_table, std::string const & buffer)
{
	_parser.parse(buffer);

	if (_parser.isError())
	{
		closeEvent(fd_table, AFdInfo::FILE_ERROR);
	}
	else if (_parser.isComplete())
	{
		closeEvent(fd_table, AFdInfo::FILE_COMPLETE);
	}
}

void CgiReader::closeEvent(FdTable & fd_table)
{
	closeEvent(fd_table, AFdInfo::FILE_COMPLETE);
}

void CgiReader::closeEvent(FdTable & fd_table, AFdInfo::Flags flag)
{
	this->flag = flag;
	updateEvents(AFdInfo::WAITING, fd_table);
	_header.swap(_parser.getHeader());
	_message_body.swap(_parser.getContent());
	_status_code = _parser.getStatusCode();
	closeFd(fd_table);
}

/* Interfacing Functions */

bool CgiReader::isChunked() const
{
	// TODO: implement check for when to send chunked request
	return false;
}

std::string const & CgiReader::getBody() const
{
	return _message_body;
}

void CgiReader::clearBody()
{
	_message_body.clear();
}

std::string CgiReader::getName() const
{
	return "CgiReader";
}
