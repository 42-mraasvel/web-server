#include "Response.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/status_codes.hpp"
#include "parser/HeaderField.hpp"
#include "parser/Request.hpp"
#include <vector>
#include <unistd.h>
#include <cstdlib>
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include <time.h>
#include <algorithm>

Response::Response(Request const & request):
_method(request.method),
_request_target(request.request_target),
_status_code(0),
_status(START),
_header_part_set(false),
_chunked(false),
_close_connection(false),
_is_cgi(false),
_config_resolver(),
_file_handler(request.method)
{
	setHttpVersion(request.minor_version);

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

void	Response::initiate(ConfigResolver::MapType const & map, Request const & request)
{
	evaluateConnectionFlag(request);
	if (validateRequest(request, false) == ERR)
	{
		return ;
	}
	if (resolveConfig(map, request) == ERR)
	{
		return ;
	}
	if (validateRequest(request, true) == ERR)
	{
		return ;
	}
	processImmdiateResponse(request);
}

/********************************************/
/****** init response - resolve config ******/
/********************************************/

int	Response::resolveConfig(ConfigResolver::MapType const & map, Request const & request)
{
	std::string host;
	if (request.header_fields.contains("host"))
	{
		host = request.header_fields.find("host")->second;
	}
	else
	{
		host = "";
	}

	_config_resolver.resolution(map, request.address, host, request.request_target);
	if (_config_resolver.result == ConfigResolver::NOT_FOUND)
	{
		markComplete(StatusCode::NOT_FOUND);
		return ERR;
	}
	if (_config_resolver.result == ConfigResolver::REDIRECT)
	{
		markComplete(_config_resolver.resolved_location->_return.first);
		processRedirectResponse(); //TODO: to move
		return OK;
	}
	if (_config_resolver.result == ConfigResolver::AUTO_INDEX_ON)
	{
		markComplete(StatusCode::STATUS_OK);
		return OK;
	}
	setEffectiveRequestURI(host, request.address.second, _config_resolver.resolved_target);
	setAbsoluteFilePath(_config_resolver.resolved_location->_root, _config_resolver.resolved_file_path);
	return OK;
}

void	Response::setEffectiveRequestURI(std::string const & resolved_host, int port, std::string const & resolved_target)
{
	std::string URI_scheme = "http://";
	std::string authority = resolved_host;
	if (port != DEFAULT_PORT)
	{
		authority += ":" + WebservUtility::itoa(port);
	}
	_effective_request_uri = URI_scheme + authority + resolved_target;
}

void	Response::setAbsoluteFilePath(std::string const & root, std::string const & resolved_file_path)
{
	if (_is_cgi)
	{
		_cgi_handler.setRootDir(root);
	}
	else
	{
		_file_handler.setAbsoluteFilePath(resolved_file_path);
	}
}

void	Response::processRedirectResponse()
{
	std::string	redirect_text = _config_resolver.resolved_location->_return.second;

	if (_status_code >= 300 && _status_code < 400)
	{
		// TODO: to check if the redirect_text needs to be absolute form??
		_effective_request_uri = redirect_text;
		_message_body = "Redirect to " + redirect_text + "\n";
	}
	else
	{
		_message_body = redirect_text;
	}
}

/**********************************************/
/****** init response - check connection ******/
/**********************************************/

void	Response::evaluateConnectionFlag(Request const & request)
{
	if (false) // TODO: change to if (request.close_connection)
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

int	Response::validateRequest(Request const & request, bool is_config_completed)
{
	if (!is_config_completed)
	{
		if (!_request_validator.isRequestValidPreConfig(request))
		{
			markComplete(_request_validator.getStatusCode());
			return ERR;
		}
	}
	else
	{
		if (!_request_validator.isRequestValidPostConfig(request, _config_resolver))
		{
			markComplete(_request_validator.getStatusCode());
			return ERR;
		}
	}
	return OK;
}
/****************************************************/
/****** (Client::readEvent) immediate response ******/
/****************************************************/

void	Response::processImmdiateResponse(Request const & request)
{
	if (isContinueResponse(request))
	{
		processContinueResponse();
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
	markComplete(StatusCode::CONTINUE);
}

/*************************************************/
/****** (Client::readEvent) execute request ******/
/*************************************************/

void	Response::executeRequest(FdTable & fd_table, Request & request)
{

	// This is the first time CGI is checked
	// Note: if request_target is "/" OR a directory: the DEFAULT index needs to be checked
	// which could also be CGI: 'index index.html index.php index.py ...'
	if (_cgi_handler.isCgi(request))
	{
		_is_cgi = true;
		if (_cgi_handler.executeRequest(fd_table, request) == ERR)
		{
			markComplete(_cgi_handler.getStatusCode());
		}
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
		if (_is_cgi && _cgi_handler.isChunked(_http_version))
		{
			// DISCUSS: (error_proof) mentioned before in comment ?
			_chunked = true;
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
	setStringToSend();
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
			_status_code = _cgi_handler.getStatusCode();
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
							+ StatusCode::getStatusMessage(_status_code);
}

void	Response::setHeaderField()
{
	setDate();
	setConnection();
	setLocation();
	setRetryAfter();
	setAllow();
	setTransferEncodingOrContentLength();
	if (!_is_cgi)
	{
		setContentType();
	}
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
	if (_status_code == StatusCode::CREATED)
	{
		_header_fields["Location"] = _request_target;
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
		for (method_iterator it = _config_resolver.resolved_location->getAllowedMethods().begin(); it != _config_resolver.resolved_location->getAllowedMethods().end(); ++it)
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
	if ( _config_resolver.result == ConfigResolver::AUTO_INDEX_ON)
	{
		_header_fields["Content-Type"] = "text/html";
		return ;
	}
	std::string file = _file_handler.getAbsoluteFilePath();
	if (_method == GET && !file.empty())
	{
		size_t	find = file.find_last_of(".");
		std::string extensin = file.substr(find);
		if (_media_type_map.contains(extensin))
		{
			_header_fields["Content-Type"] = _media_type_map.get();
			return ;
		}
		_header_fields["Content-Type"] = "application/octet-stream";
		return ;
	}
	if (!_message_body.empty())
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
/****** Client::update() ******/
/******************************/

void	Response::update(FdTable & fd_table)
{
	// handler->update(message_body, header_fields);

	// if handler->isComplete() {
	// 	// set to COMPLETE
	// } else if handler->isError() {
	// 	// errorPageCheck(fd_table);
	// }

	evaluateExecutionError();
	setMessageBody(fd_table);
	evaluateExecutionCompletion();
}

// TODO_cgi: to discuss where to put this function
void	Response::evaluateExecutionError()
{
	if (_status != COMPLETE)
	{
		if (_is_cgi)
		{
			if (_cgi_handler.evaluateExecutionError())
			{
				markComplete(_cgi_handler.getStatusCode());
			}
		}
		else
		{
			if (_file_handler.evaluateExecutionError())
			{
				markComplete(_file_handler.getStatusCode());
			}
		}
	}
}

void	Response::evaluateExecutionCompletion()
{
	if (_status != COMPLETE)
	{
		if (_is_cgi)
		{
			if (_cgi_handler.evaluateExecutionCompletion())
			{
				markComplete(_cgi_handler.getStatusCode());
			}
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

/*************************************************/
/****** Client::update() - set message body ******/
/*************************************************/

void	Response::setMessageBody(FdTable & fd_table)
{
	if (_status != COMPLETE)
	{
		setHandlerMessageBody();
	}
	else if (_message_body.empty())
	{
		if (isErrorPageRedirected(fd_table))
		{
			_status = START;
			return ;
		}
		setOtherErrorPage();
	}
}

void	Response::setHandlerMessageBody()
{
	if (_is_cgi)
	{
		// TODO_CGI Set HeaderFields
		// Discuss: can I set headerFields inside this function too?
		_cgi_handler.setMessageBody(_message_body);
	}
	else
	{
		_file_handler.setMessageBody(_message_body, _effective_request_uri);
	}
}

bool	Response::isErrorPageRedirected(FdTable & fd_table)
{
	std::string file_path;
	return _config_resolver.resolveErrorPage(_status_code, file_path) == OK
		&& _file_handler.redirectErrorPage(fd_table, file_path, _status_code) == OK;
}

void	Response::setOtherErrorPage()
{
	_message_body = WebservUtility::itoa(_status_code) + " "
					+ StatusCode::getStatusMessage(_status_code) + "\n";
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
		return _cgi_handler.isReadyToWrite();
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
