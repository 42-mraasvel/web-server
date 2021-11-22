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
		closeEvent(fd_table, AFdInfo::FILE_ERROR, StatusCode::INTERNAL_SERVER_ERROR);
		return ERR;
	}
	else if (n == 0)
	{
		closeEvent(fd_table);
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
		closeEvent(fd_table, AFdInfo::FILE_ERROR, _parser.getStatusCode());
	}
	else if (_parser.isComplete())
	{
		closeEvent(fd_table, AFdInfo::FILE_COMPLETE);
	}
}

void CgiReader::closeEvent(FdTable & fd_table)
{
	if (!_parser.isComplete() || !_parser.isCompleteIfEof())
	{
		return closeEvent(fd_table, AFdInfo::FILE_ERROR, StatusCode::BAD_GATEWAY);
	}
	closeEvent(fd_table, AFdInfo::FILE_COMPLETE);
}

void CgiReader::closeEvent(FdTable & fd_table, AFdInfo::Flags flag)
{
	closeEvent(fd_table, flag, StatusCode::STATUS_OK);
}

void CgiReader::closeEvent(FdTable & fd_table, AFdInfo::Flags flag, int status_code)
{
	this->flag = flag;
	updateEvents(AFdInfo::WAITING, fd_table);
	_header.swap(_parser.getHeader());
	_message_body.swap(_parser.getContent());
	_status_code = status_code;
	closeFd(fd_table);
}

/* Interfacing Functions */

int CgiReader::getStatusCode() const
{
	return _status_code;
}

bool CgiReader::isChunked() const
{
	// TODO: implement check for when to send chunked request
	return false;
}

HeaderField & CgiReader::getHeader()
{
	return _header;
}

std::string & CgiReader::getBody()
{
	return _message_body;
}

std::string CgiReader::getName() const
{
	return "CgiReader";
}
