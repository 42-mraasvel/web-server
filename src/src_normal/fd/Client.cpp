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
_new_response(NULL),
_response(NULL),
_close_connection(false)
{
	printf("%s-- NEW CLIENT -- %s\n", RED_BOLD, RESET_COLOR);
	printf("Client: [%s]:[%d]\n", client.first.c_str(), client.second);
	printf("Interface: [%s]:[%d]\n", interface.first.c_str(), interface.second);
}

Client::~Client()
{
	while (!_response_queue.empty())
	{
		Response*	temp = _response_queue.front();
		_response_queue.pop_front();
	}
}

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

int	Client::readEvent(FdTable & fd_table)
{
	_timer.reset();
	if (parseRequest() == ERR)
	{
		return ERR;
	}
	while (retrieveRequest())
	{
		_request->print();
		processRequest(fd_table);
		resetRequest();
	}
	return OK;
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

bool	Client::retrieveRequest()
{
	_request = _request_handler.getNextRequest();
	if (!_request)
	{
		return false;
	}
	return true;
}

void	Client::processRequest(FdTable & fd_table)
{
	if (!_new_response)
	{
		initResponse(*_request);
	}
	_new_response->executeRequest(fd_table, *_request);
}

void	Client::initResponse(Request const & request)
{
	_new_response = ResponsePointer(new Response(request));
	_response_queue.push_back(_new_response);
}

void	Client::resetRequest()
{
	_request = NULL;
	_new_response = NULL;
}

/************************/
/****** writeEvent ******/
/************************/

int	Client::writeEvent(FdTable & fd_table)
{
	_timer.reset();
	while (_response_string.size() < BUFFER_SIZE
			&& retrieveResponse())
	{
		processResponse();
		if (_response->isComplete())
		{
			_close_connection = _response->getCloseConnectionFlag();
			resetResponse();
		}
	}
	if (sendResponseString() == ERR)
	{
		closeConnection();
		return ERR;
	}
	removeWriteEvent(fd_table);
	evaluateConnection();
	return OK;
}

bool	Client::retrieveResponse()
{
	if (!_response)
	{
		if (_close_connection)
		{
			return false;
		}
		if (_response_queue.empty())
		{
			return false;
		}
		_response = _response_queue.front();
		return true;
	}
	return _response->isComplete() || _response->isReadyToWrite();
}

void	Client::processResponse()
{
	_response->generateResponse();
	appendResponseString();
}

void	Client::appendResponseString()
{
	_response_string.append(_response->getString());
	_response->clearString();
}

void	Client::evaluateConnection()
{
	if (_close_connection && _response_string.empty())
	{
		closeConnection();
	}
}

void	Client::closeConnection()
{
	std::cerr << RED_BOLD << "Connection [" << _fd << "] is set to be closed." << RESET_COLOR << std::endl;
	_flag = AFdInfo::TO_ERASE;
	_close_connection = true;
}

void	Client::resetResponse()
{
	_response_queue.pop_front();
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

/*********************/
/****** utility ******/
/*********************/
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
	for (ResponseQueue::iterator it = _response_queue.begin(); it != _response_queue.end(); ++it)
	{
		(*it)->update(fd_table);
	}
	if (!_response_string.empty()
		|| isResponseReadyToWrite())
	{
		updateEvents(AFdInfo::WRITING, fd_table);
	}

	if (!_response_queue.empty())
	{
		_timer.reset();
	}
	else if (_timer.elapsed() >= TIMEOUT)
	{
		printf("%sClient%s: TIMEOUT\n", RED_BOLD, RESET_COLOR);
		closeConnection();
	}
}

bool	Client::isResponseReadyToWrite() const
{
	return !_response_queue.empty()
			&& (_response_queue.front()->isComplete()
				|| _response_queue.front()->isReadyToWrite());
}

std::string Client::getName() const
{
	return "Client";
}
