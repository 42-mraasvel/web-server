#include "Response.hpp"
#include "settings.hpp"
#include "fd/File.hpp"
#include "utility/utility.hpp"
#include <cstdlib>
#include <fcntl.h>
#include <errno.h>
#include <iostream>

Response::Response(): _status(START), _file(NULL) {}

Response::~Response() {}

/**********************************************/
/****** (Client::readEvent) scan request ******/
/*****************************8****************/

void	Response::scanRequest(Request const & request)
{
	_method = request.method;
	setHttpVersion(request.minor_version);
	previewMethod();
	generateAbsoluteTarget(request.target_resource);
	isRequestError(request);
	immediateResponse(request);
}

void	Response::setHttpVersion(int minor_version)
{
	if (minor_version == 0)
	{
		_http_version = "HTTP/1.0";
	}
	else
	{
		_http_version = "HTTP/1.1";
	}
}

void Response::previewMethod()
{
	switch (_method)
	{
		case GET:
			_file_oflag = O_RDONLY;
			_file_event = AFdInfo::READING;
			_status_code = 200;
			break;
		case POST:
			_file_oflag = O_CREAT | O_WRONLY | O_APPEND;
			_file_event = AFdInfo::WRITING;
			_status_code = 201;
			break;
		case DELETE:
			_file_oflag = O_WRONLY;
			_file_event = AFdInfo::READING;
			_status_code = 202; //TODO: check if 204 no content
			break; 
		default:
			break;
	}
}

void	Response::generateAbsoluteTarget(std::string const & target_resource)
{
	//TODO: resort to the correct Pathname based on default path from config (add Client* client)
	if (target_resource == "/")
	{
		_absolute_target =  "./page_sample/index.html";
	}
	else
	{
		_absolute_target =  "./page_sample" + target_resource;
	}
}
bool	Response::isRequestError(Request const & request)
{
	return checkBadRequest(request.status, request.status_code)
			|| checkHttpVersion(request.major_version)
			|| checkMethod()
			|| checkExpectation(request)
			|| checkContentLength(request);
}

bool	Response::checkBadRequest(Request::RequestStatus status, int request_code)
{
	if (status == Request::BAD_REQUEST)
	{
		processError(request_code);
		return true;
	}
	return false;
}

bool	Response::checkHttpVersion(int http_major_version)
{
	if (http_major_version != 1)
	{
		processError(505); /* HTTP VERSION NOT SUPPORTED */
		return true;
	}
	return false;
}

bool	Response::checkMethod()
{
	if (_method == OTHER)
	{
		processError(501); /* NOT IMPLEMENTED */ 
		return true;
	}
	return false;
}

bool	Response::checkExpectation(Request const & request)
{
	header_iterator i = request.header_fields.find("expect");
	if (i != request.header_fields.end() && i->second != "100-continue")
	{
		processError(417); /* EXPECATION FAILED */ 
		return true;
	}
	return false;
}

bool	Response::checkContentLength(Request const & request)
{
	if (request.minor_version == 0)
	{
		header_iterator i = request.header_fields.find("content-length");
		if (i == request.header_fields.end())
		{
			processError(411); /* LENGTH REQUIRED */ 
			return true;
		}
	}
	return false;
}

/*
	Condition of sending 100 continue status code:
		- header filed: expect:100-continue
		- http version 1.1
		- content-length specified
		- no message body yet
*/
void	Response::immediateResponse(Request const & request)
{
	header_iterator i_expect = request.header_fields.find("expect");
	header_iterator i_length = request.header_fields.find("content-length");
	if (i_expect != request.header_fields.end()
		&& request.minor_version == 1
		&& i_length != request.header_fields.end()
		&& !i_length->second.empty()
		&& request.message_body.empty())
	{
		_status = COMPLETE;
		_status_code = 100; /* CONTINUE */
	}
}

/*************************************************/
/****** (Client::readEvent) execute request ******/
/*************************************************/

void	Response::executeRequest(FdTable & fd_table, Request & request)
{
	if (createFile(fd_table) == ERR)
	{
		processError(500); /* INTERNAL SERVER ERROR */
		return ;
	}
	if (executeMethod(request) == ERR)
	{
		processError(500); /* INTERNAL SERVER ERROR */
		return ;
	}
	updateFileEvent(fd_table);
}

int	Response::createFile(FdTable & fd_table)
{
	int	file_fd = open(_absolute_target.c_str(), _file_oflag, 0644);
	if (file_fd == ERR)
	{
		if (errno == ENOENT || errno == ENOTDIR)
		{
			processError(404); // NOTFOUND
		}
		else if (errno == EACCES || errno == EAGAIN || errno == EDQUOT)
		{
			processError(403); // FORBIDDEN
		}
		perror("open");
		return ERR;
	}
	printf(BLUE_BOLD "Open File:" RESET_COLOR " [%d]\n", file_fd);
	_file = new File(file_fd);
	fd_table.insertFd(_file);
	return OK;

}

int Response::executeMethod(Request & request)
{
	switch (_method)
	{
		case GET:
			return methodGet();
		case POST:
			return methodPost(request);
		case DELETE:
			return methodDelete();
		default:
			return OK;
	}
}

int	Response::methodGet()
{
	return OK;
}

int	Response::methodPost(Request & request)
{
	_file->swapContent(request.message_body);
	return OK;
}

int	Response::methodDelete()
{
	if (remove(_absolute_target.c_str()) == ERR)
	{
		if (errno == EACCES || errno == EBUSY || errno == EISDIR || errno == EPERM || errno == EROFS)
		{
			processError(403); // FORBIDDEN
		}
		perror("remove in methodDelete");
		return ERR;
	}
	printf(BLUE_BOLD "Delete File:" RESET_COLOR " [%s]\n", _absolute_target.c_str());
	return OK;
}

/****************************************************/
/****** (Client::writeEvent) generate response ******/
/****************************************************/

void	Response::generateResponse()
{
	if (_file && _file->flag == AFdInfo::FILE_ERROR)
	{
		processError(500); /* INTERNAL SERVER ERROR */
	}
	if (_status != COMPLETE) //Error status code will be already marked as COMPLETE 
	{
		switch (_method)
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
				break;
		}
		if (_file->flag == AFdInfo::FILE_COMPLETE)
		{
			_status = COMPLETE;
			deleteFile();
		}
	}
	_header_fields["Host"] = "localhost";
	if (_status == COMPLETE) // TODO: only when message_body is ready??
	{
		_header_fields["Content-Length"] = WebservUtility::itoa(_message_body.size());
	}
	setHeaderString(); //TODO: placeholder, to modify
	setResponseString(); //TODO: placeholder, to modify
	if (_status == START)
	{
		_status = HEADER_COMPLETE;
	}
}

void	Response::setHeaderString()
{
	for (header_iterator i = _header_fields.begin(); i !=_header_fields.end(); ++i)
	{
		_header_string += (i->first + ": " + i->second + NEWLINE);
	}
}

void	Response::setResponseString()
{
	if (_status == START || _status == COMPLETE)
	{
		_string_to_send = _http_version + " "
				+ WebservUtility::itoa(_status_code) + " "
				+ WebservUtility::getStatusMessage(_status_code)
				+ NEWLINE
				+ _header_string
				+ NEWLINE
				+ _message_body;
	}
	else if (_status == HEADER_COMPLETE) //TODO: to sort out where mark flag MESSAGE_BODY_ONLy
	{
		_string_to_send = _message_body;
	}
}

void	Response::responseGet()
{
	_message_body.append(_file->getContent());
	_file->clearContent();
}

void	Response::responsePost()
{
	return ;
}

void	Response::responseDelete()
{
	return ;
}

/*********************/
/****** utility ******/
/*********************/

Response::Status	Response::getStatus() const
{
	return _status;
}

int	Response::getStatusCode() const
{
	return _status_code;
}

std::string const &	Response::getString() const
{
	return _string_to_send;
}

void	Response::clearString()
{
	_string_to_send.clear();
}

void	Response::deleteFile()
{
	if (_file)
	{
		_file->flag = AFdInfo::TO_ERASE;
		_file = NULL;		
	}
}

void	Response::updateFileEvent(FdTable & fd_table)
{
	_file->updateEvents(_file_event, fd_table);
}

bool	Response::isFileComplete() const
{
	return (_file 
			&& (_file->flag == AFdInfo::FILE_COMPLETE
				|| _file->flag == AFdInfo::FILE_ERROR));
}

void	Response::processError(int error_code)
{
	if (_status != COMPLETE)
	{
		_status_code = error_code;
		_status = COMPLETE;
		generateErrorPage();
		deleteFile();
	}
}

void	Response::generateErrorPage()
{
	//TODO: to modify message
	_message_body = WebservUtility::itoa(_status_code) + " "
					+ WebservUtility::getStatusMessage(_status_code) + "\n";
}
