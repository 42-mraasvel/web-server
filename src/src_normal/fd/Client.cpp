#include "Client.hpp"
#include "settings.hpp"
#include "fd/File.hpp"
#include "utility/utility.hpp"
#include <poll.h>
#include <fcntl.h>
#include <cstdlib>

Client::Client(int fd): AFdInfo(fd), _request(NULL),  _file(NULL)
{
	_response.status_code = 0;
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
	if (readRequest() == ERR)
	{
		return ERR;
	}
	if (_request_parser.parse(_request_string) == ERR)
	{
		return ERR;
	}
	if (!_request)
	{
		_request = _request_parser.getNextRequest();
		if (!_request)
		{
			resetRequestString();
			return OK;
		}
	}
	// here _request could be below 3 status:
	//		HEADER_COMPLETE,
	//		COMPLETE,
	//		BAD_REQUEST

	if (checkErrorStatus() == true)
	{
		updateEvents(AFdInfo::WRITING, fd_table);
		//TODO: what if the status == HEADER_COMPELTE?
		return OK;
	}
	if (executeMethod(fd_table) == ERR)
	{
		return ERR;
	}

	updateEvents(AFdInfo::WAITING, fd_table);
	resetRequestString();
	return OK;

}
/**************************************/
/****** readEvent - step 1 parse ******/
/**************************************/
//TODO: Make recv work with multiple iterations, so each iter can loop over request
int	Client::readRequest()
{
	//TODO: CHECK MAXLEN
	_request_string.resize(BUFFER_SIZE, '\0');
	ssize_t ret = recv(_fd, &_request_string[0], BUFFER_SIZE, 0);
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
	_request_string.resize(ret);
	printf("Request size: %lu, Bytes read: %ld\n", _request_string.size(), ret);

	return OK;
}

/***************************************************/
/****** readEvent - step 2 check error status ******/
/***************************************************/

int	Client::checkErrorStatus()
{
	//TODO: if error code, any message_body to write?
	return (checkBadRequest()
			|| checkHttpVersion()
			|| checkMethod()
			|| checkContentLength());
}

int	Client::checkBadRequest()
{
	//TODO: to modify
	if (_request->status == Request::BAD_REQUEST)
	{
		_response.status_code = 400; /* BAD REQUEST */
		return true;
	}
	return false;
}

int	Client::checkHttpVersion()
{
	if (_request_parser.getHttpVersion().major != 1)
	{
		_response.status_code = 505; /* HTTP VERSION NOT SUPPORTED */
		return true;
	}
	return false;
}

int	Client::checkMethod()
{
	if (_request_parser.getMethod() == OTHER)
	{
		_response.status_code = 501; /* NOT IMPLEMENTED */ 
		return true;
	}
	return false;
}

int	Client::checkContentLength()
{
	if (_request_parser.getHttpVersion().minor == 0)
	{
		header_iterator i = _response.header_fields.find("content-length");
		if (i == _response.header_fields.end())
		{
			_response.status_code = 411; /* LENGTH REQUIRED */ 
			return true;
		}
	}
	return false;
}

/****************************************/
/****** readEvent - step 3 execute ******/
/****************************************/

//TODO: create response with error status
int Client::executeMethod(FdTable & fd_table)
{
	previewMethod();
	if (createFile() == ERR
		|| insertFile(fd_table) == ERR)
	{
		return ERR;
	}
	switch (_request_parser.getMethod())
	{
		case GET:
			methodGet();
			break;
		case POST:
			methodPost();
			break;
		case DELETE:
			methodDelete();
			break; 
		default:
			methodOther();
			break;
	}
	return OK;
}

void Client::previewMethod()
{
	switch (_request_parser.getMethod())
	{
		case GET:
			_oflag = O_RDONLY;
			_file_event = AFdInfo::READING;
			break;
		case POST:
			_oflag = O_CREAT | O_WRONLY | O_APPEND;
			_file_event = AFdInfo::WRITING;
			break;
		case DELETE:
			_oflag = O_RDONLY;
			_file_event = AFdInfo::READING;
			break; 
		default:
			break;
	}
}

void	Client::generateAbsoluteTarget()
{
	//TODO: resort to the correct Pathname based on default path from config (add Client* client)
	if (_request_parser.getTargetResource() == "/")
	{
		_absolute_target =  "./page_sample/index.html";
	}
	else
	{
		_absolute_target =  "./page_sample" + _request_parser.getTargetResource();
	}
}

int	Client::createFile()
{
	generateAbsoluteTarget();
	int	file_fd = open(_absolute_target.c_str(), _oflag, 0644);
	if (file_fd == ERR)
	{
		// TODO: add error handling (i.e. no file find)
		perror("open in method");
		return ERR;
	}

	printf(BLUE_BOLD "Open File:" RESET_COLOR " [%d]\n", file_fd);

	_file = new File(file_fd);

	return OK;

}

int	Client::insertFile(FdTable & fd_table)
{
	int	file_index = fd_table.size();
	if (fd_table.insertFd(_file) == ERR)
	{
		return ERR;
	}
	_file->updateEvents(_file_event, fd_table);

	return OK;
}

int	Client::methodGet()
{
	return OK;
}

int	Client::methodPost()
{
	_file->setContent(_request_parser.getMessageBody());

	return OK;
}

int	Client::methodDelete()
{

	if (remove(_request_parser.getTargetResource().c_str()) == ERR)
	{
		perror("remove in methodDelete");
		return ERR;
	}
	printf(BLUE_BOLD "Delete File:" RESET_COLOR " [%s]\n", _request_parser.getTargetResource().c_str());

	return OK;
}

int	Client::methodOther()
{
	return OK;
}

/************************/
/****** writeEvent ******/
/************************/

int	Client::writeEvent(FdTable & fd_table)
{
	generateResponse();

	if (send(_fd, _response.response.c_str(), _response.response.size(), 0) == ERR)
	{
		perror("send");
		return ERR;
	}

	updateEvents(AFdInfo::READING, fd_table);

	// TODO: reset response

	return OK;
}

void	Client::generateResponse()
{
	// TODO: 2 scenario here:
	//			1) failed status code (non-zero)
	//			2) successful status code -> to proceed on switch
	if (!_response.status_code) //For good request:
	{
		switch (_request_parser.getMethod())
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

		_file->flag = AFdInfo::TO_ERASE;
		_file = NULL;

	}
		
	_response.header_fields["Host"] = "localhost";
	_response.header_fields["Content-Length"] = WebservUtility::itoa(_response.message_body.size());

	setHttpVersion();
	setHeaderString();
	setResponse();
	
}

int	Client::responseGet()
{
	_response.status_code = 200;
	_response.message_body = _file->getContent();
	return OK;
}

int	Client::responsePost()
{
	_response.status_code = 201;
	return OK;
}

int	Client::responseDelete()
{
	//TODO: check if 204 no content
	_response.status_code = 202;
	return OK;
}

int	Client::responseOther()
{
	return OK;
}

void	Client::setHttpVersion()
{
	if (_request_parser.getHttpVersion().minor == 0)
	{
		_response.http_version = "HTTP/1.0";
	}
	else
	{
		_response.http_version = "HTTP/1.1";
	}
}

void	Client::setHeaderString()
{
	for (header_iterator i = _response.header_fields.begin(); i != _response.header_fields.end(); ++i)
	{
		_response.header_string += (i->first + ": " + i->second + NEWLINE);
	}
}

void	Client::setResponse()
{
	_response.response = _response.http_version + " "
				+ WebservUtility::itoa(_response.status_code) + " "
				+ WebservUtility::getStatusMessage(_response.status_code)
				+ NEWLINE
				+ _response.header_string
				+ NEWLINE
				+ _response.message_body;
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
bool	Client::updateEventsSpecial()
{
	return _file && _file->getEventComplete() == true;
}

//TODO: retain information about the next request if present
void	Client::resetRequestString()
{
	_request_string.clear();
}
