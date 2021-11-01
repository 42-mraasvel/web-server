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

Response::Response(Request const & request)
{
	_file = NULL;
	_status = START;
	_header_sent = false;
	_chunked = false;
	_close_connection = false;

	_method = request.method;
	_target_resource = request.target_resource;
	setHttpVersion(request.minor_version);

	// TODO: to change it properly with configuration
	_allowed_methods.push_back("GET");
	_allowed_methods.push_back("POST");
	_allowed_methods.push_back("DELETE");
	//TODO: to discuss with team
	MediaType::initMap(_media_type_map);
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

Response::~Response() {}

/*****************************************************/
/****** (Client::readEvent) scan request header ******/
/*****************************************************/

void	Response::scanRequestHeader(Request const & request)
{
	/*
	TODO: to retrieve from config class
	1. find server blocks that matches IP + Port
	2. find server blocks that match server_name with "Host" header
		- if no host or no match, choose default server
	3. inside chosen server block, find location block that match with target resource.
	4. inside the location block, take root
	*/
	std::string	default_file = "index.html";
	std::string root = "./page_sample";
	std::string	default_server = "localhost";
	std::string authority = generateAuthority(request, default_server);
	generateEffectiveRequestURI(authority);
	generateAbsoluteFilePath(root, default_file);

	previewMethod();
	if (!isRequestError(request))
	{
		continueResponse(request);
	}
}

std::string const &	Response::generateAuthority(Request const & request, std::string const & default_server)
{
	if (request.header_fields.contains("host"))
	{
		return request.header_fields.find("host")->second;
	}
	else
	{
		return default_server;
	}
}

void	Response::generateEffectiveRequestURI(std::string const & authority)
{
	std::string URI_scheme = "http://";
	_effective_request_uri = URI_scheme + authority + _target_resource;
}

void	Response::generateAbsoluteFilePath(std::string const & root, std::string const & default_file)
{
	_absolute_file_path = root + _target_resource;
	if (_target_resource.back() == '/')
	{
		_absolute_file_path += default_file;
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
	return isConnectionError(request)
			|| isBadRequest(request.status, request.status_code)
			|| isHttpVersionError(request.major_version)
			|| isMethodError()
			|| isExpectationError(request);
}

bool	Response::isConnectionError(Request const & request)
{
	// TODO: incorporate request's flag
	if (request.header_fields.contains("connection"))
	{
		if(WebservUtility::caseInsensitiveEqual(request.header_fields.find("connection")->second, "close"))
		{
			_close_connection = true;
		}
		else if (!WebservUtility::caseInsensitiveEqual(request.header_fields.find("connection")->second, "keep-alive"))
		{
			processError(400); /* BAD REQUEST */
			return true;
		}
	}
	else if (request.major_version == 1 && request.minor_version == 0)
	{
		_close_connection = true;
	}
	return false;
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
		if (access(_absolute_file_path.c_str(), F_OK) == OK)
		{
			_status_code = 204; /* NO CONTENT */
			return isFileAuthorized(access_flag);
		}
	}
	return true;
}

bool	Response::isFileExist()
{
	if (access(_absolute_file_path.c_str(), F_OK) == ERR)
	{
		processError(404); /* NOTFOUND */
		return false;
	}
	return true;
}

bool	Response::isFileAuthorized(int access_flag)
{
	if (access(_absolute_file_path.c_str(), access_flag) == ERR)
	{
		processError(403); /* FORBIDDEN */
		return false;
	}
	return true;
}

int	Response::openFile(int open_flag, FdTable & fd_table)
{
	int	file_fd = open(_absolute_file_path.c_str(), open_flag, 0644);
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
	if (remove(_absolute_file_path.c_str()) == ERR)
	{
		perror("remove");
		return ERR;
	}
	printf(BLUE_BOLD "Delete File:" RESET_COLOR " [%s]\n", _absolute_file_path.c_str());
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
	if (_status_code == 201)
	{
		_header_fields["Location"] = _target_resource;
		_message_body = "New content created!\n" + _effective_request_uri + "\n";
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
	setConnection();
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

void	Response::setConnection()
{
	if (_close_connection)
	{
		_header_fields["Connection"] = "close";
	}
	else
	{
		_header_fields["Connection"] = "keep-alive";
	}
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
		size_t	find = _absolute_file_path.find_last_of(".");
		std::string extensin = _absolute_file_path.substr(find);
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
