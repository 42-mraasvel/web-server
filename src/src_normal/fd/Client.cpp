#include "Client.hpp"
#include "settings.hpp"
#include "File.hpp"
#include "utility/utility.hpp"
#include <poll.h>
#include <fcntl.h>
#include <cstdlib>

Client::Client(int fd): AFdInfo(fd), _request(NULL), _response(NULL) {}

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
		_request = _request_parser.getNextRequest();
		if (!_request)
		{
			return OK;
		}
		if (processRequest(fd_table) == ERR)
		{
			return ERR;
		}
		_response->status = Response::COMPLETE; //TODO: to relocate correctly
		_response_queue.push(_response);
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

//TODO: Make recv work with multiple iterations, so each iter can loop over request
int	Client::readRequest(std::string & buffer)
{
	//TODO: CHECK MAXLEN
	buffer.resize(BUFFER_SIZE, '\0');
	ssize_t ret = recv(_fd, &buffer[0], BUFFER_SIZE, 0);
	if (ret == ERR)
	{
		perror("Recv");
		return (ERR);
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

/************************************************/
/****** readEvent - step 2 process request ******/
/************************************************/

int	Client::processRequest(FdTable & fd_table)
{
	_response = new Response;

	if (checkErrorStatus() == true)
	{
		_response->status = Response::COMPLETE;
	}
	else
	{
		if (executeMethod(fd_table) == ERR)
		{
			return ERR;
		}
	}
	if (generateResponse() == ERR)
	{
		return ERR;
	}
	return OK;
}


/*****************************************************/
/****** readEvent - step 2.1 check error status ******/
/*****************************************************/

bool	Client::checkErrorStatus()
{
	//TODO: if error code, any message_body to write?
	return (checkBadRequest()
			|| checkHttpVersion()
			|| checkMethod()
			|| checkContentLength());
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

/******************************************/
/****** readEvent - step 2.2 execute ******/
/******************************************/

int Client::executeMethod(FdTable & fd_table)
{
	previewMethod();
	if (createFile() == ERR
		|| insertFile(fd_table) == ERR)
	{
		return ERR;
	}
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

void Client::previewMethod()
{
	switch (_request->method)
	{
		case GET:
			_response->file_oflag = O_RDONLY;
			_response->file_event = AFdInfo::READING;
			break;
		case POST:
			_response->file_oflag = O_CREAT | O_WRONLY | O_APPEND;
			_response->file_event = AFdInfo::WRITING;
			break;
		case DELETE:
			_response->file_oflag = O_RDONLY;
			_response->file_event = AFdInfo::READING;
			break; 
		default:
			break;
	}
}

void	Client::generateAbsoluteTarget()
{
	//TODO: resort to the correct Pathname based on default path from config (add Client* client)
	if (_request->target_resource == "/")
	{
		_response->absolute_target =  "./page_sample/index.html";
	}
	else
	{
		_response->absolute_target =  "./page_sample" + _request->target_resource;
	}
}

int	Client::createFile()
{
	generateAbsoluteTarget();
	int	file_fd = open(_response->absolute_target.c_str(), _response->file_oflag, 0644);
	if (file_fd == ERR)
	{
		// TODO: add error handling (i.e. no file find)
		perror("open in method");
		return ERR;
	}
	printf(BLUE_BOLD "Open File:" RESET_COLOR " [%d]\n", file_fd);
	_response->file = new File(file_fd);
	return OK;

}

int	Client::insertFile(FdTable & fd_table)
{
	int	file_index = fd_table.size();
	if (fd_table.insertFd(_response->file) == ERR)
	{
		return ERR;
	}
	return OK;
}

int	Client::methodGet(FdTable & fd_table)
{
	_response->file->updateEvents(_response->file_event, fd_table);
	return OK;
}

int	Client::methodPost(FdTable & fd_table)
{
	// TODO: set up multiple reading/writing
	_response->file->setContent(_request->message_body);
	// TODO: check if reading is finished and ready to write into file.
	_response->file->updateEvents(_response->file_event, fd_table);
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
	_response->file->updateEvents(_response->file_event, fd_table);
	return OK;
}

int	Client::methodOther(FdTable & fd_table)
{
	return OK;
}

/***************************************************/
/****** readEvent - step 2.3 generateResponse ******/
/***************************************************/

int	Client::generateResponse()
{
	// TODO: 2 scenario here:
	//			1) failed status code (non-zero)
	//			2) successful status code -> to proceed on switch
	if (!_response->status_code) //For good request:
	{
		switch (_request->method)
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

		_response->file->flag = AFdInfo::TO_ERASE;
		_response->file = NULL;

	}
		
	_response->header_fields["Host"] = "localhost";
	if (true) // TODO: only when message_body is ready??
	{
		_response->header_fields["Content-Length"] = WebservUtility::itoa(_response->message_body.size());
	}
	setHttpVersion(); //TODO: placeholder, to modify
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

void	Client::setHttpVersion()
{
	if (_request->minor_version == 0)
	{
		_response->http_version = "HTTP/1.0";
	}
	else
	{
		_response->http_version = "HTTP/1.1";
	}
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
/****** writeEvent ******/
/************************/

int	Client::writeEvent(FdTable & fd_table)
{
	if (_response_queue.empty())
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
