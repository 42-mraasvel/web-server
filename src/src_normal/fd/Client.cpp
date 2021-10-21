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
/**************************************/
/****** readEvent - step 1 parse ******/
/**************************************/
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

/*************************************************/
/****** readEvent - step 2 retrieve request ******/
/*************************************************/

bool	Client::retrieveRequest()
{
	_request = _request_parser.getNextRequest();
	if (!_request)
	{
		return false;
	}
	return true;
}

/************************************************/
/****** readEvent - step 3 process request ******/
/************************************************/

int	Client::processRequest(FdTable & fd_table)
{
	initResponse();
	if (isRequestError() == true)
	{
		_new_response->status = Response::COMPLETE;
		updateEvents(AFdInfo::WRITING, fd_table);
		return OK;
	}
	if (setupFile(fd_table) == ERR)
	{
		return ERR;
	}
	if (executeMethod(fd_table) == ERR)
	{
		return ERR;
	}
	_new_response->file->updateEvents(_new_response->file_event, fd_table);
	return OK;
}

/************************************************/
/****** readEvent - step 3.1 init response ******/
/************************************************/

void	Client::initResponse()
{
	_new_response = new Response(*_request);
	_response_queue.push(_new_response);
}

/******************************************************/
/****** readEvent - step 3.2 check request error ******/
/******************************************************/

//TODO: to pick up pre-defined status code from request
bool	Client::isRequestError()
{
	//TODO: if error code, any message_body to write?
	return checkBadRequest()
			|| checkHttpVersion()
			|| checkMethod()
			|| checkContentLength();
}

bool	Client::checkBadRequest()
{
	if (_request->status == Request::BAD_REQUEST)
	{
		_new_response->status_code = _request->status_code;
		return true;
	}
	return false;
}

bool	Client::checkHttpVersion()
{
	if (_request->major_version != 1)
	{
		_new_response->status_code = 505; /* HTTP VERSION NOT SUPPORTED */
		return true;
	}
	return false;
}

bool	Client::checkMethod()
{
	if (_request->method == OTHER)
	{
		_new_response->status_code = 501; /* NOT IMPLEMENTED */ 
		return true;
	}
	return false;
}

bool	Client::checkContentLength()
{
	if (_request->minor_version == 0)
	{
		header_iterator i = _request->header_fields.find("content-length");
		if (i == _request->header_fields.end())
		{
			_new_response->status_code = 411; /* LENGTH REQUIRED */ 
			return true;
		}
	}
	return false;
}

/*********************************************/
/****** readEvent - step 3.3 setup file ******/
/*********************************************/

int	Client::setupFile(FdTable & fd_table)
{
	if (_new_response->createFile() == ERR)
	{
		return ERR;
	}
	int	file_index = fd_table.size();
	if (fd_table.insertFd(_new_response->file) == ERR)
	{
		return ERR;
	}
	return OK;
}

/******************************************/
/****** readEvent - step 3.4 execute ******/
/******************************************/

int Client::executeMethod(FdTable & fd_table)
{
	switch (_new_response->method)
	{
		case GET:
			methodGet(fd_table);
			break;
		case POST:
			methodPost(fd_table);
			break;
		case DELETE:
			methodDelete(fd_table);
			break; 
		default:
			methodOther(fd_table);
			break;
	}
	return OK;
}

int	Client::methodGet(FdTable & fd_table)
{
	return OK;
}

int	Client::methodPost(FdTable & fd_table)
{
	_new_response->file->swapContent(_request->message_body);
	return OK;
}

int	Client::methodDelete(FdTable & fd_table)
{
	if (remove(_request->target_resource.c_str()) == ERR)
	{
		perror("remove in methodDelete");
		return ERR;
	}
	printf(BLUE_BOLD "Delete File:" RESET_COLOR " [%s]\n", _request->target_resource.c_str());
	return OK;
}

//TODO: unnecessary??
int	Client::methodOther(FdTable & fd_table)
{
	return OK;
}

/**********************************************/
/****** readEvent - step 4 reset request ******/
/**********************************************/

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
	_response_string.append(_response->string_to_send);
	_response->string_to_send.clear();
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
	if (_response->status == Response::COMPLETE)
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
	// when the top response's File event starts reading (GET) or finishes writing (POST), will mark Client as ready for WRITING
	if (!_response_queue.empty()
		&& _response_queue.front()->file
		&& (_response_queue.front()->file->flag == AFdInfo::FILE_START
			|| _response_queue.front()->file->flag == AFdInfo::FILE_COMPLETE))
	{
		updateEvents(AFdInfo::WRITING, fd_table);
	}
}
