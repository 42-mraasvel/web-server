#include "Response.hpp"
#include "settings.hpp"
#include "fd/File.hpp"
#include "utility/utility.hpp"
#include "parser/HeaderField.hpp"
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <time.h>
#include <algorithm>

Response::Response()
{
	_file = NULL;
	_status = START;
	_header_sent = false;
	_chunked = false;
	_close_connection = false;

	// TODO: to change it properly with configuration
	_allowed_methods.push_back("GET");
	_allowed_methods.push_back("POST");
	_allowed_methods.push_back("DELETE");
	//TODO: to discuss with team
	MediaType::initMap(_media_type_map);
}

Response::~Response() {}

/*****************************************************/
/****** (Client::readEvent) scan request header ******/
/*****************************************************/

void	Response::scanRequestHeader(Request const & request)
{
	_method = request.method;
	setCloseConnectionFlag(request);
	setHttpVersion(request.minor_version);
	generateAbsoluteTarget(request.target_resource);
	previewMethod();
	if (!isRequestError(request))
	{
		continueResponse(request);
	}
}

void	Response::setCloseConnectionFlag(Request const & request)
{
	// TODO: to incorporate with reqeust
	if (true)
	{
		_close_connection = true;
	}
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

void Response::previewMethod()
{
	switch (_method)
	{
		case GET:
			_status_code = 200;
			return;
		case POST:
			_status_code = 201;
			return;
		case DELETE:
			_status_code = 204;
			return;
		default:
			return;
	}
}

bool	Response::isRequestError(Request const & request)
{
	return isBadRequest(request.status, request.status_code)
			|| isHttpVersionError(request.major_version)
			|| isMethodError()
			|| isExpectationError(request);
}

bool	Response::isBadRequest(Request::RequestStatus status, int request_code)
{
	if (status == Request::BAD_REQUEST)
	{
		processError(request_code);
		return true;
	}
	return false;
}

bool	Response::isHttpVersionError(int http_major_version)
{
	if (http_major_version != 1)
	{
		processError(505); /* HTTP VERSION NOT SUPPORTED */
		return true;
	}
	return false;
}

bool	Response::isMethodError()
{
	if (_method == OTHER)
	{
		processError(501); /* NOT IMPLEMENTED */ 
		return true;
	}
	if (!findMethod(_method))
	{
		processError(405); /* METHOD NOT ALLOWED */ 
		return true;		
	}
	return false;
}

bool	Response::findMethod(MethodType const method) const
{
	std::string	method_string;
	switch (method)
	{
		case GET:
			method_string = "GET";
			break ;
		case POST:
			method_string = "POST";
			break ;
		case DELETE:
			method_string = "DELETE";
			break ;
		default:
			method_string = "OTHER";
	}
	method_const_iterator	it = std::find(_allowed_methods.begin(), _allowed_methods.end(), method_string);
	return it != _allowed_methods.end();
}

bool	Response::isExpectationError(Request const & request)
{
	if (request.header_fields.contains("expect") &&
		!WebservUtility::caseInsensitiveEqual(request.header_fields.find("expect")->second, "100-continue"))
	{
		processError(417); /* EXPECATION FAILED */ 
		return true;
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
void	Response::continueResponse(Request const & request)
{
	if (request.header_fields.contains("expect")
		&& request.minor_version == 1
		&& request.header_fields.contains("content-length")
		&& !(request.header_fields.find("content-length")->second.empty())
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
	int	access_flag;
	int	open_flag;
	setFileParameter(access_flag, open_flag);

	if (!isFileReady(access_flag))
	{
		return ERR;
	}
	return openFile(open_flag, fd_table);
}

void	Response::setFileParameter(int & access_flag, int & open_flag)
{
	switch(_method)
	{
		case GET:
			access_flag = R_OK;
			open_flag = O_RDONLY;
			_file_event = AFdInfo::READING;
			return ;
		case POST:
			access_flag = W_OK;
			open_flag = O_CREAT | O_WRONLY | O_APPEND;
			_file_event = AFdInfo::WRITING;
			return ;
		case DELETE:
			access_flag = W_OK;
			open_flag = O_WRONLY;
			_file_event = AFdInfo::WAITING;
			return ;
		case OTHER:
		defaul:
			return;
	}
}

bool	Response::isFileReady(int access_flag)
{
	if (_method == GET || _method == DELETE)
	{
		return isFileExist() && isFileAuthorized(access_flag);
	}
	if (_method == POST)
	{
		if (access(_absolute_target.c_str(), F_OK) == OK)
		{
			_status_code = 204; /* NO CONTENT */
			return isFileAuthorized(access_flag);
		}
	}
	return true;
}

bool	Response::isFileExist()
{
	if (access(_absolute_target.c_str(), F_OK) == ERR)
	{
		processError(404); /* NOTFOUND */
		return false;
	}
	return true;
}

bool	Response::isFileAuthorized(int access_flag)
{
	if (access(_absolute_target.c_str(), access_flag) == ERR)
	{
		processError(403); /* FORBIDDEN */
		return false;
	}
	return true;
}

int	Response::openFile(int open_flag, FdTable & fd_table)
{
	int	file_fd = open(_absolute_target.c_str(), open_flag, 0644);
	if (file_fd == ERR)
	{
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
			return executeGet();
		case POST:
			return executePost(request);
		case DELETE:
			return executeDelete();
		default:
			return OK;
	}
}

int	Response::executeGet()
{
	return OK;
}

int	Response::executePost(Request & request)
{
	_file->swapContent(request.message_body);
	return OK;
}

int	Response::executeDelete()
{
	if (remove(_absolute_target.c_str()) == ERR)
	{
		perror("remove");
		return ERR;
	}
	printf(BLUE_BOLD "Delete File:" RESET_COLOR " [%s]\n", _absolute_target.c_str());
	_file->flag = AFdInfo::FILE_COMPLETE;
	return OK;
}

/********************************************************/
/****** (Client::writeEvent) pre-generate response ******/
/********************************************************/

void	Response::defineEncoding()
{
	if (_status != COMPLETE
		&& _http_version == "HTTP/1.1"
		&& _method == GET)
	{
		if (_file
			&& _file->flag != AFdInfo::FILE_ERROR
			&& _file->getContent().size() >= BUFFER_SIZE)
		{
			_chunked = true;
		}
	}
}

void	Response::checkFileError()
{
	if (_status != COMPLETE && _file && _file->flag == AFdInfo::FILE_ERROR)
	{
		processError(500); /* INTERNAL SERVER ERROR */
	}
}

/****************************************************/
/****** (Client::writeEvent) generate response ******/
/***************************************************/

void	Response::generateResponse()
{
	if (_status != COMPLETE)
	{
		responseMethod();
		checkFileComplete();
	}
	setStringToSent();
}

void	Response::responseMethod()
{
	switch (_method)
	{
		case GET:
			return responseGet();
		case POST:
			return responsePost();
		case DELETE:
			return responseDelete();
		default:
			return ;
	}
}

void	Response::responseGet()
{
	_message_body.append(_file->getContent());
	_file->clearContent();
}

void	Response::responsePost()
{
	//TODO: fill in URI-reference
	if (_status_code == 201)
	{
		_header_fields["Location"] = _absolute_target;
	}
	return ;
}

void	Response::responseDelete()
{
	return ;
}

void	Response::checkFileComplete()
{
	if (_status != COMPLETE && _file && _file->flag == AFdInfo::FILE_COMPLETE)
	{
		_status = COMPLETE;
		deleteFile();
	}
}

void	Response::setStringToSent()
{
	if (_chunked)
	{
		doChunked();
	}
	else
	{
		noChunked();
	}
}

void	Response::doChunked()
{
	if (!_header_sent)
	{		
		setHeader();
		_header_sent = true;
	}
	if (_header_sent)
	{
		encodeMessageBody();
		_string_to_send.append(_message_body);
		_message_body.clear();
	}
}

void	Response::encodeMessageBody()
{
	if (!_message_body.empty())
	{
		std::string chunk_size = WebservUtility::itoa(_message_body.size(), 16) + NEWLINE;
		_message_body.insert(0, chunk_size);
		_message_body.append(NEWLINE);
	}
	if (_status == COMPLETE)
	{
		_message_body.append(CHUNK_TAIL);
	}
}

void	Response::setHeader()
{
	setStringStatusLine();
	setDate();
	setRetryAfter();
	setAllow();
	setTransferEncodingOrContentLength();
	setContentType();
	setStringHeader();
	_string_to_send = _string_status_line + NEWLINE
					+ _string_header + NEWLINE;

}

void	Response::setStringStatusLine()
{
	_string_status_line = _http_version + " "
							+ WebservUtility::itoa(_status_code) + " "
							+ WebservUtility::getStatusMessage(_status_code);
}

void	Response::setDate()
{
	char		buf[1000];
	time_t		now = time(0);
	struct tm	tm = *gmtime(&now);
	strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S %Z", &tm);
	
	_header_fields["Data"] = std::string(buf);
}

void	Response::setRetryAfter()
{
	if (_status_code >= 300 && _status_code < 400)
	{
		_header_fields["Retry-After"] = WebservUtility::itoa(RETRY_AFTER_SECONDS);
	}
}

void	Response::setAllow()
{
	if (_status_code == 405)
	{
		std::string	value;
		for (method_iterator it = _allowed_methods.begin(); it != _allowed_methods.end(); ++it)
		{
			value.append(*it + ", ");
		}
		if (!value.empty())
		{
			value.erase(value.size() - 2, 2);
		}
		_header_fields["Allow"] = value;
	}
}

void	Response::setTransferEncodingOrContentLength()
{
	if (_chunked)
	{
		_header_fields["Transfer-Encoding"] = "chunked";
	}
	else
	{
		setContentLength();
	}

}

void	Response::setContentLength()
{
	if ((_status_code >= 100 && _status_code < 200)
		|| _status_code == 204)
	{
		return ;
	}
	_header_fields["Content-Length"] = WebservUtility::itoa(_message_body.size());
}

void	Response::setContentType()
{
	if (_method == GET && _status_code == 200)
	{
		size_t	find = _absolute_target.find_last_of(".");
		std::string extensin = _absolute_target.substr(find);
		if (_media_type_map.contains(extensin))
		{
			_header_fields["Content-Type"] = _media_type_map.get();
		}
		else
		{
			_header_fields["Content-Type"] = "application/octet-stream";
		}
	}
	else if (!_message_body.empty())
	{
		_header_fields["Content-Type"] = "text/plain;charset=UTF-8";
	}
}

void	Response::setStringHeader()
{
	for (header_iterator i = _header_fields.begin(); i !=_header_fields.end(); ++i)
	{
		_string_header += (i->first + ": " + i->second + NEWLINE);
	}
}

void	Response::noChunked()
{
	if (_status == COMPLETE)
	{
		setHeader();
		_string_to_send.append(_message_body);
	}
}

/*********************/
/****** utility ******/
/*********************/

Response::Status	Response::getStatus() const
{
	return _status;
}

bool	Response::getCloseConnectionFlag() const
{
	return _close_connection;
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

bool	Response::isFileReady() const
{
	return _file 
			&& (_file->flag == AFdInfo::FILE_COMPLETE
				|| _file->flag == AFdInfo::FILE_ERROR
				|| (_file->flag == AFdInfo::FILE_START
					&& !_file->getContent().empty()));
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
