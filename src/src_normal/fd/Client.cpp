#include "Client.hpp"
#include "settings.hpp"
#include "File.hpp"
#include "utility/utility.hpp"
#include <poll.h>
#include <fcntl.h>
#include <cstdlib>
#include <algorithm>

Client::Client(int fd): AFdInfo(fd), _request(NULL), _new_response(NULL), _response(NULL) {}

Client::~Client()
{
	while (!_response_queue.empty())
	{
		Response*	temp = _response_queue.front();
		_response_queue.pop();
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
	if (parseRequest() == ERR)
	{
		return ERR;
	}
	while (retrieveRequest())
	{
		initResponse();
		_new_response->executeRequest(fd_table, *_request);
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
	_request = _request_parser.getNextRequest();
	if (!_request)
	{
		return false;
	}
	return true;
}

void	Client::initResponse()
{
	_new_response = new Response();
	_new_response->scanRequest(*_request);
	_response_queue.push(_new_response);
}

void	Client::resetRequest()
{
	delete _request;
	_request = NULL;
	_new_response = NULL;
}

/************************/
/****** writeEvent ******/
/************************/

int	Client::writeEvent(FdTable & fd_table)
{
	while (_response_string.size() < BUFFER_SIZE)
	{
		if (retrieveResponse() == false)
		{
			updateEvents(AFdInfo::READING, fd_table);
			break;
		}
		_response->generateResponse();
		appendResponseString();
		resetResponse();
	}
	if (sendResponseString() == ERR)
	{
		closeConnection();
		return ERR;
	}
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
	}
	return true;
}

void	Client::appendResponseString()
{
	_response_string.append(_response->getString());
	_response->clearString();
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

void	Client::resetResponse()
{
	if (_response->getStatus() == Response::COMPLETE)
	{
		delete _response;
		_response_queue.pop();
		_response = NULL;
	}
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
	if (flag == AFdInfo::TO_ERASE)
	{
		printf(BLUE_BOLD "Close File:" RESET_COLOR " [%d]\n", _fd);
		fd_table.eraseFd(_index);
	}
	/*
	mark Client as ready for WRITING when:
	1. the top response's is complete (Error)
	2. File event starts reading (GET) or finishes writing (POST),
	*/
	if (!_response_queue.empty()
		&& (_response_queue.front()->getStatus() == Response::COMPLETE
			|| _response_queue.front()->isFileStart()))
	{
		updateEvents(AFdInfo::WRITING, fd_table);
	}
}

void	Client::closeConnection()
{
	flag = AFdInfo::TO_ERASE;
	std::cerr << RED_BOLD << "Socket error, close connect [" << _fd << "]." << RESET_COLOR << std::endl;
}
