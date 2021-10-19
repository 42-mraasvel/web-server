#include "Client.hpp"
#include "settings.hpp"
#include "File.hpp"
#include "utility/utility.hpp"
#include <poll.h>
#include <fcntl.h>
#include <cstdlib>

Client::Client(int fd): AFdInfo(fd), _request(NULL), _response(NULL) {}

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
	while (true)
	{
		if (_request) // message_body reading in progress (assume only applies for Post hence append directly to File.Content)
		{
			appendFileContent();
		}
		else // ready to process new request
		{
			if (retrieveRequest() == false)
			{
				return OK;
			}
			if (processRequest(fd_table) == ERR)
			{
				return ERR;
			}
		}
		if (_request->status == Request::COMPLETE)
		{
			resetRequest();
		}
		else
		{
			return OK;
		}
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
	buffer.clear();
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
	_response->file->updateEvents(_response->file_event, fd_table);
	return OK;
}

/************************************************/
/****** readEvent - step 3.1 init response ******/
/************************************************/

void	Client::initResponse()
{
	_response = new Response(*_request);
	_response_queue.push(_response);
}

/******************************************************/
/****** readEvent - step 3.2 check request error ******/
/******************************************************/

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
	//TODO: to modify
	if (_request->status == Request::BAD_REQUEST)
	{
		_response->status_code = 400; /* BAD REQUEST */
		return true;
	}
	return false;
}

bool	Client::checkHttpVersion()
{
	if (_request->major_version != 1)
	{
		_response->status_code = 505; /* HTTP VERSION NOT SUPPORTED */
		return true;
	}
	return false;
}

bool	Client::checkMethod()
{
	if (_request->method == OTHER)
	{
		_response->status_code = 501; /* NOT IMPLEMENTED */ 
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
			_response->status_code = 411; /* LENGTH REQUIRED */ 
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
	if (_response->createFile() == ERR)
	{
		return ERR;
	}
	int	file_index = fd_table.size();
	if (fd_table.insertFd(_response->file) == ERR)
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
	switch (_request->method)
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
	appendFileContent();
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
	_response = NULL;
}

/************************/
/****** writeEvent ******/
/************************/

int	Client::writeEvent(FdTable & fd_table)
{
	if (_response_queue.empty())
	{
		return ERR;
	}
	if (generateResponse() == ERR)
	{
		return ERR;
	}
	Response*	response = _response_queue.front();
	if (send(_fd, response->string.c_str(), response->string.size(), 0) == ERR)
	{
		perror("send");
		return ERR;
	}
	if (response->status == Response::COMPLETE)
	{
		_response_queue.pop();
		updateEvents(AFdInfo::READING, fd_table);
	}
	return OK;
}
/***************************************************/
/****** writeEvent - step 2.3 generateResponse ******/
/***************************************************/

int	Client::generateResponse()
{
	// TODO: 2 scenario here:
	//			1) failed status code (non-zero)
	//			2) successful status code -> to proceed on switch
	if (!_response->status_code) //For good request:
	{
		switch (_response->method)
		{
			case GET:
				responseGet();
				break;
			case POST:
				responsePost();
				break;
			case DELETE:
				responseDelete();
				break; 
			default:
				responseOther();
				break;
		}

		_response->file->flag = AFdInfo::TO_ERASE; //TODO: move
		_response->file = NULL; //TODO: move

	}
		
	_response->header_fields["Host"] = "localhost";
	if (true) // TODO: only when message_body is ready??
	{
		_response->header_fields["Content-Length"] = WebservUtility::itoa(_response->message_body.size());
	}
	setHeaderString(); //TODO: placeholder, to modify
	setResponseString(); //TODO: placeholder, to modify
	return OK;
}

int	Client::responseGet()
{
	_response->status_code = 200;
	//TODO: set up multiple sending
	_response->message_body = _response->file->getContent();
	return OK;
}

int	Client::responsePost()
{
	_response->status_code = 201;
	return OK;
}

int	Client::responseDelete()
{
	//TODO: check if 204 no content
	_response->status_code = 202;
	return OK;
}

int	Client::responseOther()
{
	return OK;
}

void	Client::setHeaderString()
{
	for (header_iterator i = _response->header_fields.begin(); i != _response->header_fields.end(); ++i)
	{
		_response->header_string += (i->first + ": " + i->second + NEWLINE);
	}
}

void	Client::setResponseString()
{
	// TODO: check if start of the message of part of message body;
	_response->string = _response->http_version + " "
			+ WebservUtility::itoa(_response->status_code) + " "
			+ WebservUtility::getStatusMessage(_response->status_code)
			+ NEWLINE
			+ _response->header_string
			+ NEWLINE
			+ _response->message_body;
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

bool	Client::updateEventsSpecial()
{
	return !_response_queue.empty() && _response_queue.front()->file && _response_queue.front()->file->getEventComplete() == true;
}

void	Client::appendFileContent()
{
	_response->file->appendContent(_request->message_body);
	_request->message_body.erase();
}
