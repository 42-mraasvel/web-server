#include "Connection.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/status_codes.hpp"
#include "File.hpp"
#include "handler/Response.hpp"

#include <poll.h>
#include <fcntl.h>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <unistd.h>

Connection::Connection(int fd, AddressType client,
	AddressType interface, Config::address_map const * config_map):
AFdInfo(fd),
_request_handler(client, interface, config_map),
_request(NULL),
_response(NULL),
_close_connection(false),
_close_timer_set(false),
_unsafe_request_count(0)
{
	PRINT_INFO << YELLOW_BOLD "-- New Connection --" RESET_COLOR << std::endl;
	PRINT_INFO << "Connection: [" << client.first << "]:[" << client.second << "]" << std::endl;
	PRINT_INFO << "Interface: [" << interface.first << "]:[" << client.second << "]" << std::endl;

}

Connection::~Connection() {}

struct pollfd	Connection::getPollFd() const
{
	struct pollfd temp;
	temp.fd = _fd;
	temp.events = POLLIN;
	temp.revents = 0;
	return temp;
}

/***********************/
/****** readEvent ******/
/***********************/

void	Connection::readEvent(FdTable & fd_table)
{
	_timer.reset();
	parseRequest();
}

int	Connection::parseRequest()
{
	std::string	buffer;
	if (readRequest(buffer) == ERR)
	{
		closeConnection();
		return ERR;
	}
	_request_handler.parse(buffer);
	return OK;
}

int	Connection::readRequest(std::string & buffer)
{
	buffer.resize(BUFFER_SIZE, '\0');
	ssize_t ret = recv(_fd, &buffer[0], BUFFER_SIZE, 0);
	if (ret == ERR)
	{
		syscallError(_FUNC_ERR("recv"));
		return ERR;
	}
	else if (ret == 0)
	{
		return ERR;
	}
	buffer.resize(ret);
	PRINT_DEBUG << getName() << ": [" << getFd() << "]: Read " << ret << " bytes, Request size: " << buffer.size() << std::endl;
	return OK;
}

/*************************/
/****** updateEvent ******/
/*************************/

void	Connection::update(FdTable & fd_table)
{
	executeRequests(fd_table);

	_response_handler.updateResponseQueue(fd_table);

	generateResponseString();

	resetEvents(fd_table);

	checkTimeOut();
}

void Connection::executeRequests(FdTable & fd_table)
{
	while (canExecuteRequest(fd_table.size())
			&& retrieveRequest())
	{
		_request->print();
		increUnsafe(_request->method);
		_response_handler.processRequest(fd_table, *_request);
		resetRequest();
	}
}

bool Connection::canExecuteRequest(int fd_table_size) const
{
	return !_close_connection
			&& !_unsafe_request_count
			&& _response_handler.canExecuteRequest()
			&& !(!_request_handler.isNextRequestSafe()
			&& !_response_handler.isResponseQueueEmpty())
			&& fd_table_size < FD_TABLE_MAX_SIZE;
}

bool	Connection::retrieveRequest()
{
	_request = _request_handler.getNextRequest();
	return _request;
}

void	Connection::resetRequest()
{
	_request = NULL;
}

static void	appendString(std::string & from, std::string & to)
{
	to.append(from);
	from.clear();
}

void	Connection::generateResponseString()
{
	while (_response_string.size() < BUFFER_SIZE
			&& retrieveResponse())
	{
		appendString(_response->string_to_send, _response_string);
		if (_response->isFinished())
		{
			decreUnsafe(_response->method);
			_close_connection = _response->close_connection;
			resetResponse();
		}
	}
}

bool	Connection::retrieveResponse()
{
	if (!_response)
	{
		if (_close_connection)
		{
			return false;
		}
		_response = _response_handler.getNextResponse();
		if (!_response)
		{
			return false;
		}
	}
	return !_response->string_to_send.empty();
}

void	Connection::resetResponse()
{
	_response = NULL;
	_response_handler.popQueue();
}

void	Connection::resetEvents(FdTable & fd_table)
{
	if (getFlag() == AFdInfo::TO_ERASE)
	{
		updateEvents(AFdInfo::WAITING, fd_table);
	}
	else if (_close_connection)
	{
		removeEvents(AFdInfo::READING, fd_table);
	}
	else
	{
		if (_request_handler.numRequests() >= REQUEST_QUEUE_THRESHOLD)
		{
			removeEvents(AFdInfo::READING, fd_table);
		}
		else
		{
			addEvents(AFdInfo::READING, fd_table);
		}
	}
	if (!_response_string.empty())
	{
		addEvents(AFdInfo::WRITING, fd_table);
	}
	else
	{
		removeEvents(AFdInfo::WRITING, fd_table);
	}
}

void	Connection::checkTimeOut()
{
	if (_close_timer_set && _timer.elapsed() >= CLOSE_CONNECTION_DELAY)
	{
		closeConnection();
		return ;
	}
	if (!_response_handler.isResponseQueueEmpty())
	{
		_timer.reset();
	}
	else if (_timer.elapsed() >= TIMEOUT && !_close_connection)
	{
		PRINT_INFO << getName() << ": [" << getFd() << "]: Timeout" << std::endl;
		_request_handler.newTimeoutRequest();
	}
}

/************************/
/****** writeEvent ******/
/************************/

void	Connection::writeEvent(FdTable & fd_table)
{
	_timer.reset();
	if (sendResponseString() == ERR)
	{
		closeConnection();
		return;
	}
	evaluateConnection(fd_table);
}

int	Connection::sendResponseString()
{
	if (!_response_string.empty())
	{
		size_t size = std::min((size_t)BUFFER_SIZE, _response_string.size());
#ifdef __APPLE__
		if (send(_fd, _response_string.c_str(), size, 0) == ERR)
#else
		if (send(_fd, _response_string.c_str(), size, MSG_NOSIGNAL) == ERR)
#endif
		{
			syscallError(_FUNC_ERR("send"));
			return ERR;
		}
		_response_string.erase(0, size);
	}
	return OK;
}

void	Connection::evaluateConnection(FdTable & fd_table)
{
	if (_close_connection && _response_string.empty())
	{
		_timer.reset();
		_close_timer_set = true;
	}
}

/****************************/
/****** exceptionEvent ******/
/****************************/

void	Connection::exceptionEvent(FdTable & fd_table)
{
	AFdInfo::exceptionEvent(fd_table); // RM, REMOVE, just for printing purposes
	_response_handler.clear();
	_request_handler.clear();
	closeConnection();
}

/*********************/
/****** utility ******/
/*********************/

void	Connection::closeConnection()
{
	PRINT_INFO << BLUE_BOLD << getName() << RESET_COLOR ": [" << getFd() << "] is set to be closed." << RESET_COLOR << std::endl;
	setFlag(AFdInfo::TO_ERASE);
}

bool	Connection::isMethodSafe(Method::Type const & method) const
{
	return method == Method::GET;
}

void	Connection::increUnsafe(Method::Type const & method)
{
	if (!isMethodSafe(method))
	{
		_unsafe_request_count++;
	}
}

void	Connection::decreUnsafe(Method::Type const & method)
{
	if (!isMethodSafe(method))
	{
		_unsafe_request_count--;
	}
}

/***********************/
/****** debugging ******/
/***********************/

std::string Connection::getName() const
{
	return "Connection";
}

