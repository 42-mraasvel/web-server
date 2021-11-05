#include "Response.hpp"
#include "settings.hpp"
#include "fd/File.hpp"
#include "utility/utility.hpp"
#include "parser/HeaderField.hpp"
#include <cstdlib>
#include <fcntl.h>
#include <errno.h>
#include <iostream>

Response::Response(): _status(START), _file(NULL), _header_sent(false), _chunked(false) {}

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
	if (_status != COMPLETE)
	{
		continueResponse(request);
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
			_file_event = AFdInfo::WAITING;
			_status_code = 200;
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
			|| checkExpectation(request);
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
	if (_cgi_handler.isCgi(&request)) {
		_cgi_handler.execute(&request, fd_table);
		_is_cgi = true;
		_file = NULL;
		return ;
	}
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
		if (errno == EACCES || errno == EBUSY || errno == EISDIR || errno == EPERM || errno == EROFS)
		{
			processError(403); /* FORBIDDEN */
		}
		perror("remove in methodDelete");
		return ERR;
	}
	printf(BLUE_BOLD "Delete File:" RESET_COLOR " [%s]\n", _absolute_target.c_str());
	_file->flag = AFdInfo::FILE_COMPLETE;
	return OK;
}

/***************************************************/
/****** (Client::writeEvent) prepare to write ******/
/***************************************************/

void	Response::prepareToWrite()
{
	checkFileError();
	defineEncoding();
}

void	Response::checkFileError()
{
	if (_status != COMPLETE && _file && _file->flag == AFdInfo::FILE_ERROR)
	{
		processError(500); /* INTERNAL SERVER ERROR */
	}
}

void	Response::defineEncoding()
{
	if (_status != COMPLETE
		&& _http_version == "HTTP/1.1"
		&& _method == GET)
	{
		if (_is_cgi && _cgi_handler.getContent().size() >= BUFFER_SIZE)
		{
			_chunked = true;
		}
		else if (_file && _file->getContent().size() >= BUFFER_SIZE)
		{
			_chunked = true;
		}
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
	if (_is_cgi) {
		_message_body.append(_cgi_handler.getContent());
		_cgi_handler.clearContent();
	} else {
		_message_body.append(_file->getContent());
		_file->clearContent();
	}
}

void	Response::responsePost()
{
	//TODO: fill in URI-reference
	_header_fields["Location"] = _absolute_target;
	_message_body = "New content is created on " + _absolute_target;
	return ;
}

void	Response::responseDelete()
{
	_message_body = "Target delete successfully!\n";
	return ;
}

void	Response::checkFileComplete()
{
	if (_is_cgi && _status != COMPLETE && _cgi_handler.getStatus() == CgiHandler::COMPLETE)
	{
		_status = COMPLETE;
	}
	else if (_status != COMPLETE && _file && _file->flag == AFdInfo::FILE_COMPLETE)
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
		_header_fields["Transfer-Encoding"] = "chunked";
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
		setContentLength();
		setHeader();
		_string_to_send.append(_message_body);
	}
}

void	Response::setContentLength()
{
	if (_header_fields.contains("transfer-encoding")
		|| (_status_code >= 100 && _status_code < 200)
		|| _status_code == 204)
	{
		return ;
	}
	_header_fields["Content-Length"] = WebservUtility::itoa(_message_body.size());
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

bool	Response::isFileReady() const
{
	if (_is_cgi) {
		return _cgi_handler.isComplete();
	}

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
