#include "Client.hpp"
#include "settings.hpp"
#include "fd/File.hpp"
#include "utility/utility.hpp"
#include <poll.h>
#include <fcntl.h>
#include <cstdlib>


Client::Client(int fd): AFdInfo(fd), _file(NULL)
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
	if (parseRequest() == ERR)
	{
		return ERR;
	}
	if (checkError() == ERR)
	{
		return ERR;
	}
	if (_response.status_code)
	{
		updateEvents(AFdInfo::WRITING, fd_table);
		return OK;
	}
	if (!_response.status_code && executeMethod(fd_table) == ERR)
	{
		return ERR;
	}
	// TODO: figure out the correct location for this function.
	updateEvents(AFdInfo::WAITING, fd_table);
	
	return OK;

}
/**************************************/
/****** readEvent - step 1 parse ******/
/**************************************/
//TODO: Make recv work with multiple iterations, so each iter can loop over request
int	Client::parseRequest()
{
	//TODO: CHECK MAXLEN
	_request.resize(BUFFER_SIZE, '\0');
	ssize_t ret = recv(_fd, &_request[0], BUFFER_SIZE, 0);
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
	_request.resize(ret);
	printf("Request size: %lu, Bytes read: %ld\n", _request.size(), ret);

	//TODO: change it to 1) continue reading 2) BAD_REQUEST 3) TO_EXECUTE (incoprotate updatedEvent())
	if(_request_parser.parseHeader(_request) == RequestParser::BAD_REQUEST)
	{
		return (ERR);
	}

	generateAbsoluteTarget();

	return OK;
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
/********************************************/
/****** readEvent - step 2 check error ******/
/********************************************/

int	Client::checkError()
{
	checkHttpVersion();
	checkMethod();
	checkContentLength();
	return OK;
}

void	Client::checkHttpVersion()
{
	if (_request_parser.getHttpVersion().major != 1)
	{
		//TODO: check if anything should be added on the header field or message body?
		_response.status_code = 505; /* HTTP VERSION NOT SUPPORTED */
	}
}

void	Client::checkMethod()
{
	if (!_response.status_code && _request_parser.getMethod() == RequestParser::OTHER)
	{
		//TODO: check if anything should be added on the header field or message body?
		_response.status_code = 501; /* NOT IMPLEMENTED */ 
	}
}

void	Client::checkContentLength()
{
	if (!_response.status_code &&_request_parser.getHttpVersion().minor == 0)
	{
		header_iterator i = _response.header_fields.find("content-length");
		if (i == _response.header_fields.end())
		{
			//TODO: check if anything should be added on the header field or message body?
			_response.status_code = 411; /* LENGTH REQUIRED */ 
		}
	}
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
		case RequestParser::GET:
			methodGet();
			break;
		case RequestParser::POST:
			methodPost();
			break;
		case RequestParser::DELETE:
			methodDelete();
			break; 
		default:
			methodOther();
			break;
	}
	resetBuffer(); //for _request
	return OK;
}

void Client::previewMethod()
{
	switch (_request_parser.getMethod())
	{
		case RequestParser::GET:
			_oflag = O_RDONLY;
			_file_event = AFdInfo::READING;
			break;
		case RequestParser::POST:
			_oflag = O_CREAT | O_WRONLY | O_APPEND;
			_file_event = AFdInfo::WRITING;
			break;
		case RequestParser::DELETE:
			_oflag = O_RDONLY;
			_file_event = AFdInfo::READING;
			break; 
		default:
			break;
	}
}

int	Client::createFile()
{
	int	file_fd = open(_absolute_target.c_str(), _oflag, 0644);
	if (file_fd == ERR)
	{
		// TODO: add error handling (i.e. no file find)
		perror("open in method");
		return ERR;
	}

	printf(BLUE_BOLD "Open File:" RESET_COLOR " [%d]\n", file_fd);

	_file = new File(this, file_fd);

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


//TODO: retain information about the next request if present
void	Client::resetBuffer()
{
	_request.clear();
}

/************************/
/****** writeEvent ******/
/************************/

int	Client::writeEvent(FdTable & fd_table)
{
	generateResponse();

	if (_file) //TODO: to clean out later
	{
		_file->flag = AFdInfo::TO_ERASE;
		_file = NULL;
	}
	if (send(_fd, _response.response.c_str(), _response.response.size(), 0) == ERR)
	{
		perror("send");
		return ERR;
	}

	updateEvents(AFdInfo::READING, fd_table);

	return OK;
}

void	Client::generateResponse()
{
	if (!_response.status_code) //TODO: to clean out later
	{
		switch (_request_parser.getMethod())
		{
			case RequestParser::GET:
				responseGet();
				break;
			case RequestParser::POST:
				responsePost();
				break;
			case RequestParser::DELETE:
				responseDelete();
				break; 
			default:
				responseOther();
				break;
		}
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
