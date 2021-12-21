#include "handler/CgiHandler.hpp"
#include "settings.hpp"
#include "utility/Output.hpp"
#include "utility/utility.hpp"
#include "utility/macros.hpp"
#include "utility/status_codes.hpp"
#include <libgen.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <sys/wait.h>

CgiHandler::CgiHandler()
: _status(CgiHandler::INACTIVE), _status_code(0), _sender(NULL), _reader(NULL), _cgi_pid(-1) {}

CgiHandler::~CgiHandler()
{
	destroyFds();
}

bool CgiHandler::isCgi(std::string const & request_target, CgiVectorType const & cgi)
{
	for (CgiVectorType::const_iterator it = cgi.begin(); it != cgi.end(); ++it)
	{
		if (WebservUtility::stringEndsWith(request_target, it->first))
		{
			return true;
		}
	}
	return false;
}

bool CgiHandler::isCgi(Request const & request)
{
	if (request.config_info.result == ConfigInfo::NOT_FOUND)
	{
		return false;
	}
	return isCgi(request.config_info.resolved_target, request.config_info.resolved_location->_cgi);
}

/*
1. Preparation: ScriptLocation, MetaVariables

	- ScriptLocation: based off Configuration options
	- MetaVariables

2. Open pipes to connect STDIN and STDOUT, create new instances
3. Fork and execute the script, store the PID internally
4. Close unused pipe ends
*/
int CgiHandler::executeRequest(FdTable& fd_table, Request& request)
{
	PRINT_DEBUG << YELLOW_BOLD "-- Executing CGI --" RESET_COLOR << std::endl;
	/* 1. Preparation */
	_status = CgiHandler::INCOMPLETE;

	int cgi_fds[2] = {-1, -1};
	if (initializeCgiConnection(cgi_fds, fd_table, request) == ERR)
	{
		finishCgi(ERROR, StatusCode::INTERNAL_SERVER_ERROR);
		return ERR;
	}

	if (_executor.execute(fd_table, request, cgi_fds) == ERR)
	{
		WebservUtility::closePipe(cgi_fds);
		finishCgi(ERROR, _executor.getStatusCode());
		return ERR;
	}


	WebservUtility::closePipe(cgi_fds);
	_timer.reset();
	return OK;
}

/*
Initializes the CgiReader, CgiSender classes
Stores the FDs used inside of the CGI in rfds
*/
int CgiHandler::initializeCgiConnection(int* cgi_fds, FdTable& fd_table, Request& r)
{
	if (initializeCgiReader(cgi_fds, fd_table) == ERR)
	{
		return ERR;
	}

	if (initializeCgiSender(cgi_fds, fd_table, r) == ERR)
	{
		if (close(cgi_fds[1]) == ERR)
		{
			syscallError(_FUNC_ERR("close"));
		}
		return ERR;
	}
	return OK;
}

int CgiHandler::initializeCgiReader(int* cgi_fds, FdTable& fd_table)
{
	int fds[2];

	if (pipe(fds) == ERR)
	{
		return syscallError(_FUNC_ERR("pipe"));
	}

	// Reader needs the READ end of the pipe, so it gives the WRITE end to the CGI
	cgi_fds[1] = fds[1];
	// Instantiate the CgiReader with the READ end of the pipe and add it to the FdTable.
	if (WebservUtility::makeNonBlocking(fds[0]) == ERR)
	{
		WebservUtility::closePipe(fds);
		return syscallError(_FUNC_ERR("fcntl"));
	}
	try
	{
		_reader = SmartPointer<CgiReader>(new CgiReader(fds[0], &_timer));
		fd_table.insertFd(SmartPointer<AFdInfo>(_reader));
	}
	catch (...)
	{
		WebservUtility::closePipe(fds);
		_reader = NULL;
		throw;
	}
	return OK;
}

int CgiHandler::initializeCgiSender(int* cgi_fds, FdTable& fd_table, Request& r)
{
	int fds[2];

	if (pipe(fds) == ERR)
	{
		return syscallError(_FUNC_ERR("pipe"));
	}

	// Sender needs the WRITE end of the pipe, so it gives the READ end to the CGI
	cgi_fds[0] = fds[0];

	// Instantiate the CgiSender with the WRITE end of the pipe and add it to the FdTable
	if (WebservUtility::makeNonBlocking(fds[1]) == ERR)
	{
		WebservUtility::closePipe(fds);
		return syscallError(_FUNC_ERR("fcntl"));
	}
	try
	{
		_sender = SmartPointer<CgiSender>(new CgiSender(fds[1], &r, &_timer));
		fd_table.insertFd(SmartPointer<AFdInfo>(_sender));
	}
	catch (...)
	{
		WebservUtility::closePipe(fds);
		WebservUtility::closePipe(cgi_fds);
		_sender = NULL;
		throw;
	}
	return OK;
}

bool CgiHandler::isExecutionError() const
{
	return (_reader && _reader->getFlag() == AFdInfo::ERROR)
		|| (_sender && _sender->getFlag() == AFdInfo::ERROR);
}

int CgiHandler::getErrorCode() const
{
	if (_reader && _reader->getFlag() == AFdInfo::ERROR)
	{
		return _reader->getStatusCode();
	}
	
	return _sender->getStatusCode();
}

/*
This function should only be called once
*/
void CgiHandler::setSpecificHeaderField(HeaderField & header_field)
{
	swapHeader();
	for (HeaderField::const_iterator it = _header.begin(); it != _header.end(); ++it)
	{
		// Don't add Content-Length if TE is present
		if (WebservUtility::caseInsensitiveEqual(it->first, "content-length")
			&& header_field.contains("Transfer-Encoding"))
		{
			continue;
		}
		if (!skippedHeaderField(it->first))
		{
			header_field[it->first] = it->second;
		}
	}
	_header.clear();
}

bool CgiHandler::skippedHeaderField(std::string const & key) const
{
	static const std::string skipped_fields[] = {
		"status"
	};

	for (std::size_t i = 0; i < sizeof(skipped_fields) / sizeof(skipped_fields[0]); ++i)
	{
		if (WebservUtility::caseInsensitiveEqual(skipped_fields[i], key))
		{
			return true;
		}
	}
	return false;
}

/*
Function's purpose:
	- Check if ERROR occured, CGI child process status (still alive etc)
		IF exited: wait
		IF exited AND sender is INCOMPLETE: BAD_GATEWAY
	- Check if either Sender or Reader is complete: remove from table if so
	- Set completion status if both are complete
*/
void CgiHandler::update(std::string & response_body)
{
	//return if already finished communicating with CGI
	if (isComplete() || isError())
	{
		return;
	}

	if (isExecutionError())
	{
		finishCgi(CgiHandler::ERROR, getErrorCode());
		return;
	}

	if (_reader)
	{
		evaluateReader(response_body);
	}

	if (_sender && _sender->getFlag() == AFdInfo::COMPLETE)
	{
		_sender->setToErase();
		_sender = NULL;
	}

	if (isComplete())
	{
		finishCgi(CgiHandler::COMPLETE, _status_code);
	}
	else if (_timer.elapsed() >= TIMEOUT)
	{
		PRINT_INFO << RED_BOLD "CgiHandler" RESET_COLOR ": Timeout" << std::endl;
		finishCgi(CgiHandler::ERROR, StatusCode::GATEWAY_TIMEOUT);
	}
}

void CgiHandler::evaluateReader(std::string & response_body)
{
	if (_reader->getBody().size() > 0)
	{
		swapHeader();
		if (response_body.size() == 0)
		{
			response_body.swap(_reader->getBody());
		}
		else
		{
			response_body.append(_reader->getBody());
			_reader->getBody().clear();
		}
	}
	if (_reader->getFlag() == AFdInfo::COMPLETE)
	{
		swapHeader();
		_reader->setToErase();
		_reader = NULL;
	}
}

void CgiHandler::swapHeader()
{
	if (_reader && _header.size() == 0)
	{
		_header.swap(_reader->getHeader());
		_status_code = checkStatusField();
	}
}

void CgiHandler::exceptionEvent()
{
	finishCgi(CgiHandler::ERROR, StatusCode::INTERNAL_SERVER_ERROR);
	PRINT_ERR << "CgiHandler: exceptionEvent" << std::endl;
}

int CgiHandler::checkStatusField() const
{
	HeaderField::const_pair_type status = _header.get("Status");
	if (status.second)
	{
		PRINT_DEBUG << "Cgi returned status:  " << status.first->second << std::endl;
		return WebservUtility::strtol(status.first->second);
	}
	return StatusCode::STATUS_OK;
}

/* Utilities */

bool CgiHandler::isComplete() const
{
	return _status != CgiHandler::INACTIVE && _sender == NULL && _reader == NULL;
}

bool CgiHandler::isError() const
{
	return _status == CgiHandler::ERROR;
}

void CgiHandler::finishCgi(Status status, int code)
{
	_executor.clear();
	destroyFds();
	_status = status;
	_status_code = code;
}

/* Clean up, destroying */

void CgiHandler::destroyFds()
{
	if (_sender)
	{
		_sender->setToErase();
		_sender = NULL;
	}

	if (_reader)
	{
		_reader->setToErase();
		_reader = NULL;
	}
}

/* Getters */

int CgiHandler::getStatusCode() const
{
	return _status_code;
}
