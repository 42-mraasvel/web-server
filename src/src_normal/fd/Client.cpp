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

void	Client::updateEvents(AFdInfo::EventTypes type, FdTable & fd_table)
{
	short int updated_events;

	switch (type)
	{
		case AFdInfo::READING:
			updated_events = POLLIN;
			break;
		case AFdInfo::WRITING:
			updated_events = POLLOUT;
			break;
		case AFdInfo::WAITING:
			updated_events = 0;
			break;
	}
	fd_table[_index].first.events = updated_events | POLLIN;
}

void	Client::update(FdTable & fd_table)
{
	executeRequests(fd_table);

	_response_handler.updateResponseQueue(fd_table);

	if (!_response_string.empty()
		|| _response_handler.canClientWrite())
	{
		updateEvents(AFdInfo::WRITING, fd_table);
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

void Client::executeRequests(FdTable & fd_table)
{
	while (canExecuteRequest() && retrieveRequest())
	{
		_request->print();
		increUnsafe(_request->method);
		_response_handler.processRequest(fd_table, *_request);
		resetRequest();
	}
}

bool Client::canExecuteRequest() const
{
	return !_unsafe_request_count
		&& !(!_request_handler.isNextRequestSafe() && !_response_handler.isResponseQueueEmpty());
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

/************************/
/****** writeEvent ******/
/************************/

// TODO: to discuss: should we move this working _response to ResponseHandler??
void	Client::writeEvent(FdTable & fd_table)
{
	_timer.reset();
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
	if (sendResponseString() == ERR)
	{
		closeConnection();
		return;
	}
	removeWriteEvent(fd_table);
	evaluateConnection();
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

void	Client::evaluateConnection()
{
	if (_close_connection && _response_string.empty())
	{
		closeConnection();
	}
}

void	Client::resetResponse()
{
	_response = NULL;
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

void	Client::removeWriteEvent(FdTable & fd_table)
{
	updateEvents(AFdInfo::READING, fd_table);
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
	_close_connection = true;
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

