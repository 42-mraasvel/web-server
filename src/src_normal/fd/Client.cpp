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

Client::Client(int fd, Address address): AFdInfo(fd), _address(address), _request(NULL), _new_response(NULL), _response(NULL) {}

Client::~Client()
{
	while (!_response_queue.empty())
	{
		Response*	temp = _response_queue.front();
		_response_queue.pop_front();
		delete temp;
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
	//TODO_config: add config map parameter, pass it on to processRequest
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
	_request_parser.parse(buffer);
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
	if (!_request)
	{
		_request = _request_parser.getNextRequest();
		if (!_request)
		{
			return false;
		}
		_request->address = _address; //TODO: to discuss where to put this
		return true;
	}
	else
	{
		if (_request->status == Request::COMPLETE)
		{
			return true;
		}
		return false;
	}
}

void	Client::processRequest(FdTable & fd_table)
{
	//TODO_config: add config map parameter, pass it on to initResponse
	if (!_new_response)
	{
		initResponse(*_request);
	}
	if (isRequestReadyToExecute())
	{
		_new_response->executeRequest(fd_table, *_request);
	}
}

void	Client::initResponse(Request const & request)
{
	//TODO_config: add config map parameter, pass it on to response->initative
	_new_response = new Response(request);
	_response_queue.push_back(_new_response);
	_new_response->initiate(request);
}

bool	Client::isRequestReadyToExecute() const
{
	return _request->status == Request::COMPLETE
			&& !isRequestExecuted();
}

bool	Client::isRequestExecuted() const
{
	return _new_response
			&& _new_response->isComplete()
			&& _new_response->getStatusCode() != StatusCode::CONTINUE;
}

void	Client::resetRequest()
{
	if (_request->status == Request::COMPLETE)
	{
		delete _request;
		_request = NULL;
		_new_response = NULL;
	}
	else if (_new_response
			&& _new_response->isComplete()
			&& _new_response->getStatusCode() == StatusCode::CONTINUE)
	{
		_new_response = NULL;
	}
}

/************************/
/****** writeEvent ******/
/************************/

int	Client::writeEvent(FdTable & fd_table)
{
	while (_response_string.size() < BUFFER_SIZE
			&& retrieveResponse())
	{
		processResponse();
		if (_response->isComplete())
		{
			evaluateConnection();
			resetResponse();
		}
	}
	if (sendResponseString() == ERR)
	{
		closeConnection();
		return ERR;
	}
	updateEvents(AFdInfo::READING, fd_table);
	return OK;
}

bool	Client::retrieveResponse()
{
	if (!_response)
	{
		if (_response_queue.empty())
		{
			return false;
		}
		_response = _response_queue.front();
		_response->defineEncoding();
		return true;
	}

	return _response->isHandlerReadyToWrite();
}

void	Client::processResponse()
{
	_response->generateResponse();
	if (flag != AFdInfo::TO_ERASE)
	{
		appendResponseString();
	}
}

void	Client::appendResponseString()
{
	_response_string.append(_response->getString());
	_response->clearString();
}

void	Client::evaluateConnection()
{
	if (_response->getCloseConnectionFlag())
	{
		closeConnection();
	}
}

void	Client::closeConnection()
{
	if (flag != AFdInfo::TO_ERASE)
	{
		std::cerr << RED_BOLD << "Connection [" << _fd << "] is set to be closed." << RESET_COLOR << std::endl;
		flag = AFdInfo::TO_ERASE;
	}
}

void	Client::resetResponse()
{
	delete _response;
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
	for (ResponseQueue::const_iterator it = _response_queue.begin(); it != _response_queue.end(); ++it)
	{
		(*it)->update(fd_table);
	}
	if (!_response_string.empty()
		|| isResponseReadyToWrite())
	{
		updateEvents(AFdInfo::WRITING, fd_table);
	}
}

bool	Client::isResponseReadyToWrite() const
{
	return !_response_queue.empty()
			&& (_response_queue.front()->isComplete()
				|| _response_queue.front()->isHandlerReadyToWrite());
}

std::string Client::getName() const
{
	return "Client";
}
