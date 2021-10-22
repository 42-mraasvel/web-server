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
		if (processRequest(fd_table) == ERR)
		{
			return ERR;
		}
		resetRequest();
	}
	return OK;

}

int	Client::parseRequest()
{
	std::string	buffer;
	if (readRequest(buffer) == ERR)
	{
		return ERR;
	}
	if (_request_parser.parse(buffer) == ERR)
	{
		return ERR;
	}
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
		flag = AFdInfo::TO_ERASE;
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

int	Client::processRequest(FdTable & fd_table)
{
	initResponse();
	if (_new_response->executeRequest(fd_table, *_request) == ERR)
	{
		return ERR;
	}
	return OK;
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
		if (processResponse() == ERR)
		{
			return ERR;
		}
		resetResponse();
	}
	if (sendResponseString() == ERR)
	{
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

int	Client::processResponse()
{
	if (_response->generateResponse() == ERR)
	{
		return ERR;
	}
	appendResponseString();
	return OK;
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

/************************/
/****** closeEvent ******/
/************************/

int	Client::closeEvent()
{
	//TODO:
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
