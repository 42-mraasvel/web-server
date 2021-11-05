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

Response::Response(Request const & request): _file_handler(request.method)
{
	_status = START;
	_header_part_set = false;
	_chunked = false;
	_close_connection = false;
	_is_cgi = false;

	_status_code = 0;
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

/************************************************/
/****** (Client::readEvent) init response  ******/
/************************************************/

void	Response::initiate(Request const & request)
{
	resolveConfig(request);
	evaluateConnectionFlag(request);
	if (validateRequest(request) == ERR)
	{
		return ;
	}
	processImmdiateResponse(request);
}

/********************************************/
/****** init response - resolve config ******/
/********************************************/

void	Response::resolveConfig(Request const & request)
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
	std::string	root = "./page_sample";
	std::string	default_server = "localhost";
	std::string	authority = setAuthority(request, default_server);
	setEffectiveRequestURI(authority);
	setAbsoluteFilePath(root, default_file);
}

std::string const &	Response::setAuthority(Request const & request, std::string const & default_server)
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

void	Response::setEffectiveRequestURI(std::string const & authority)
{
	std::string URI_scheme = "http://";
	_effective_request_uri = URI_scheme + authority + _target_resource;
}

void	Response::setAbsoluteFilePath(std::string const & root, std::string const & default_file)
{
	_absolute_file_path = root + _target_resource;
	if (_target_resource[_target_resource.size() - 1] == '/')
	{
		_absolute_file_path += default_file;
	}
	if (_is_cgi)
	{
		// TODO_CGI: _cgi_handler.setAbsoluteFilePath(_absolute_file_path);
	}
	else
	{
		_file_handler.setAbsoluteFilePath(_absolute_file_path);
	}
}

/**********************************************/
/****** init response - check connection ******/
/**********************************************/

void	Response::evaluateConnectionFlag(Request const & request)
{
	if (false) // TODO_CGI: change to if (request.close_connection)
	{
		_close_connection = true;
	}
	else if (request.header_fields.contains("connection"))
	{
		if(WebservUtility::caseInsensitiveEqual(request.header_fields.find("connection")->second, "close"))
		{
			_close_connection = true;
		}
	}
	else if (request.major_version == 1 && request.minor_version == 0)
	{
		_close_connection = true;
	}
}

/************************************************/
/****** init response - validate request  *******/
/************************************************/

int	Response::validateRequest(Request const & request)
{
	if (!_request_validator.isRequestValid(request))
	{
		markComplete(_request_validator.getStatusCode());
		return ERR;
	}
	return OK;
}
/****************************************************/
/****** (Client::readEvent) immediate response ******/
/****************************************************/

void	Response::processImmdiateResponse(Request const & request)
{
	if (isRedirectResponse())
	{
		processRedirectResponse();
	}
	else if (isContinueResponse(request))
	{
		processContinueResponse();
	}
}

bool	Response::isRedirectResponse() const
{
	// TODO: to incorporate config
	bool	redirect_flag = false;
	return redirect_flag;
}

void	Response::processRedirectResponse()
{
	// TODO: to incorporate config
	int			redirect_code = 301;
	std::string	redirect_text = "http://this_is_the_redirect_url.com";

	markComplete(redirect_code);
	if (_status_code >= 300 && _status_code < 400)
	{
		_effective_request_uri = redirect_text;
		_message_body = "Redirect to " + redirect_text + "\n";
	}
	else
	{
		_message_body = redirect_text;
	}
}

bool	Response::isContinueResponse(Request const & request) const
{
	return request.header_fields.contains("expect")
			&& request.minor_version == 1
			&& request.header_fields.contains("content-length")
			&& !(request.header_fields.find("content-length")->second.empty())
			&& request.message_body.empty();
}

void	Response::processContinueResponse()
{
	markComplete(100);
}

/*************************************************/
/****** (Client::readEvent) execute request ******/
/*************************************************/

void	Response::executeRequest(FdTable & fd_table, Request & request)
{
	if (_is_cgi)
	{
		// TODO_CGI: add _cgi_handler.executeRequest(): markComplete if there is error (clean cgi fd if error);
	}
	else
	{
		if (_file_handler.executeRequest(fd_table, request) == ERR)
		{
			markComplete(_file_handler.getStatusCode());
		}
	}
}

/********************************************************/
/****** (Client::writeEvent) pre-generate response ******/
/********************************************************/

void	Response::defineEncoding()
{
	if (_status != COMPLETE)
	{
		if (_is_cgi)
		{
			// TODO_CGI:: add cgi check chunekd (error proof)
		}
		else if (_file_handler.isChunked(_http_version))
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
	evaluateExecutionError();
	setMessageBody();
	evaluateExecutionCompletion();
	setStringToSend();
}

void	Response::evaluateExecutionError()
{
	if (_status != COMPLETE)
	{
		if (_is_cgi)
		{
			// TODO_CGI _cgi_handler.evaluateExecutionError() (1.give status code (should be 500), 2.clean cgi fd if error)
		}
		else
		{
			if (_file_handler.evaluateExecutionError())
			{
				markComplete(500); /* INTERNAL SERVER ERROR */
			}
		}
	}
}

void	Response::setMessageBody()
{
	if (_status != COMPLETE)
	{
		setHandlerMessageBody();
	}
	else
	{
		setErrorPage();
	}
}

void	Response::setHandlerMessageBody()
{
	if (_is_cgi)
	{
		// TODO_CGI _cgi_handler.setMessageBody(_message_body, ...)
	}
	else
	{
		_file_handler.setMessageBody(_message_body, _effective_request_uri);
	}
}

void	Response::setErrorPage()
{
	//TODO: to modify message
	_message_body = WebservUtility::itoa(_status_code) + " "
					+ WebservUtility::getStatusMessage(_status_code) + "\n";
}

void	Response::evaluateExecutionCompletion()
{
	if (_status != COMPLETE)
	{
		if (_is_cgi)
		{
			// TODO_CGI: _cgi_handler.evaluateExecutionCompletion() (clean cgi fd if complete)
		}
		else
		{
			if (_file_handler.evaluateExecutionCompletion())
			{
				markComplete(_file_handler.getStatusCode());
			}
		}
	}
}

/****************************************************/
/****** generate response - set string to send ******/
/****************************************************/

void	Response::setStringToSend()
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

void	Response::noChunked()
{
	if (_status == COMPLETE)
	{
		setHeaderPart();
		_string_to_send.append(_message_body);
	}
}

void	Response::doChunked()
{
	if (!_header_part_set)
	{		
		setHeaderPart();
	}
	if (_header_part_set)
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

void	Response::setHeaderPart()
{
	setStatusCode();
	setStringStatusLine();
	setHeaderField();
	setStringHeaderField();
	_string_to_send = _string_status_line + NEWLINE
					+ _string_header_field + NEWLINE;

	_header_part_set = true;
}

void	Response::setStatusCode()
{
	if (_status != COMPLETE)
	{
		if (_is_cgi)
		{
			// TODO_CGI _cgi_handler.getStatusCode();
		}
		else
		{
			_status_code = _file_handler.getStatusCode();
		}
	}
}

void	Response::setStringStatusLine()
{
	_string_status_line = _http_version + " "
							+ WebservUtility::itoa(_status_code) + " "
							+ WebservUtility::getStatusMessage(_status_code);
}

void	Response::setHeaderField()
{
	setDate();
	setConnection();
	setLocation();
	setRetryAfter();
	setAllow();
	setTransferEncodingOrContentLength();
	setContentType();
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

void	Response::setLocation()
{
	if (_status_code == 201)
	{
		_header_fields["Location"] = _target_resource;
	}
	else if (_status_code >= 300 && _status_code < 400)
	{
		_header_fields["Location"] = _effective_request_uri;
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

void	Response::setStringHeaderField()
{
	for (header_iterator i = _header_fields.begin(); i !=_header_fields.end(); ++i)
	{
		_string_header_field += (i->first + ": " + i->second + NEWLINE);
	}
}

/******************************/
/****** utility - public ******/
/******************************/

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

bool	Response::isComplete() const
{
	return _status == COMPLETE;
}

bool	Response::isHandlerReadyToWrite() const
{
	if (_is_cgi)
	{
		// TODO_CGI return CGI
		return true;
	}
	else
	{
		return _file_handler.isFileReadyForResponse();
	}
}

/*******************************/
/****** utility - private ******/
/*******************************/

void	Response::markComplete(int code)
{
	_status = COMPLETE;
	_status_code = code;
}
