#include "Client.hpp"
#include "settings.hpp"
#include "File.hpp"
#include "utility/utility.hpp"
#include "utility/status_codes.hpp"
#include <poll.h>
#include <fcntl.h>
#include <cstdlib>
#include <algorithm>
#include <iostream>

Client::Client(int fd, AddressType client,
	AddressType interface, Config::address_map const * config_map):
AFdInfo(fd),
_request_handler(client, interface, config_map),
_request(NULL),
_response(NULL),
_close_connection(false),
_close_timer_set(false),
_unsafe_request_count(0)
{
	printf("%s-- NEW CLIENT -- %s\n", RED_BOLD, RESET_COLOR);
	printf("Client: [%s]:[%d]\n", client.first.c_str(), client.second);
	printf("Interface: [%s]:[%d]\n", interface.first.c_str(), interface.second);
}

Client::~Client() {}

struct pollfd	Client::getPollFd() const
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

void	Client::readEvent(FdTable & fd_table)
{
	_timer.reset();
	parseRequest();
}

int	Client::parseRequest()
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

int	Client::readRequest(std::string & buffer)
{
	buffer.resize(BUFFER_SIZE, '\0');
	ssize_t ret = recv(_fd, &buffer[0], BUFFER_SIZE, 0);
	if (ret == ERR)
	{
		perror("Recv");
		return ERR;
	}
	else if (ret == 0)
	{
		return ERR;
	}
	buffer.resize(ret);
	printf("Request size: %lu, Bytes read: %ld\n", buffer.size(), ret);
	return OK;
}

/*************************/
/****** updateEvent ******/
/*************************/

void	Client::update(FdTable & fd_table)
{
	if (_flag == AFdInfo::TO_ERASE)
	{
		return ;
	}

	executeRequests(fd_table);

	_response_handler.updateResponseQueue(fd_table);

	generateResponse();

	resetEvents(fd_table);

	checkTimeOut();
}

void Client::executeRequests(FdTable & fd_table)
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

bool Client::canExecuteRequest(int fd_table_size) const
{
	return !_close_connection
			&& !_unsafe_request_count
			&& !(!_request_handler.isNextRequestSafe()
				&& !_response_handler.isResponseQueueEmpty())
			&& fd_table_size < FD_TABLE_MAX_SIZE;
}

bool	Client::retrieveRequest()
{
	_request = _request_handler.getNextRequest();
	return _request;
}

void	Client::resetRequest()
{
	_request = NULL;
}

void	Client::generateResponse()
{
	while (_response_string.size() < BUFFER_SIZE
			&& retrieveResponse())
	{
		_response->generateResponse(_response_string);
		if (_response->isComplete())
		{
			decreUnsafe(_response->getMethod());
			_close_connection = _response->getCloseConnectionFlag();
			resetResponse();
		}
	}
}

bool	Client::retrieveResponse()
{
	if (!_response)
	{
		if (_close_connection)
		{
			return false;
		}
		_response = _response_handler.getNextResponse();
		return _response;
	}
	return _response->isComplete() || _response->isReadyToWrite();
}

void	Client::resetResponse()
{
	_response = NULL;
	_response_handler.popQueue();
}

void	Client::resetEvents(FdTable & fd_table)
{
	if (_flag == AFdInfo::TO_ERASE)
	{
		updateEvents(AFdInfo::WAITING, fd_table);
	}
	else if (_close_connection)
	{
		removeEvents(AFdInfo::READING, fd_table);
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

void	Client::checkTimeOut()
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
	else if (_timer.elapsed() >= TIMEOUT)
	{
		printf("%sClient%s: [%d]: TIMEOUT\n", RED_BOLD, RESET_COLOR, getFd());
		closeConnection();
	}
}

/************************/
/****** writeEvent ******/
/************************/

void	Client::writeEvent(FdTable & fd_table)
{
	_timer.reset();
	if (sendResponseString() == ERR)
	{
		closeConnection();
		return;
	}
	evaluateConnection(fd_table);
}

int	Client::sendResponseString()
{
	if (!_response_string.empty())
	{
		size_t size = std::min((size_t)BUFFER_SIZE, _response_string.size());
		if (send(_fd, _response_string.c_str(), size, 0) == ERR)
		{
			perror("send");
			return ERR;
		}
		_response_string.erase(0, size);
	}
	return OK;
}

void	Client::evaluateConnection(FdTable & fd_table)
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

void	Client::exceptionEvent(FdTable & fd_table)
{
	AFdInfo::exceptionEvent(fd_table); // RM, REMOVE, just for printing purposes
	_response_handler.clear();
	_request_handler.clear();
	closeConnection();
}

/*********************/
/****** utility ******/
/*********************/

void	Client::closeConnection()
{
	std::cerr << RED_BOLD << "Connection [" << _fd << "] is set to be closed." << RESET_COLOR << std::endl;
	_flag = AFdInfo::TO_ERASE;
	// TODO: close the connection only in the update after a certain amount of time has elapsed: remove READING from events
}

bool	Client::isMethodSafe(Method::Type const & method) const
{
	return method == Method::GET;
}

void	Client::increUnsafe(Method::Type const & method)
{
	if (!isMethodSafe(method))
	{
		_unsafe_request_count++;
	}
}

void	Client::decreUnsafe(Method::Type const & method)
{
	if (!isMethodSafe(method))
	{
		_unsafe_request_count--;
	}
}

/***********************/
/****** debugging ******/
/***********************/

std::string Client::getName() const
{
	return "Client";
}

