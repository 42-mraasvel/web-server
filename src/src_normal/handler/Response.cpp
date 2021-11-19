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
_config_info(request.config_info),
_status(START),
_header_part_set(false),
_chunked(false),
_close_connection(request.close_connection),
_is_cgi(false),
_file_handler(request.method)
{
	setHttpVersion(request.minor_version);
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

/*************************************************/
/****** (Client::readEvent) execute request ******/
/*************************************************/

void	Response::executeRequest(FdTable & fd_table, Request & request)
{
	switch(request.status)
	{
		case Request::BAD_REQUEST:
			markComplete(request.status_code);
			break ;
		case Request::EXPECT:
			markComplete(StatusCode::CONTINUE);
			break ;
		case Request::COMPLETE:
			processCompleteRequest(fd_table, request);
			break ;
		default:
			return ;
	}
}

/**********************************************************/
/****** execute request - process complete request   ******/
/**********************************************************/

void	Response::processCompleteRequest(FdTable & fd_table, Request & request)
{
	processCgiRequest(request);
	switch(_config_info.result)
	{
		case ConfigInfo::REDIRECT:
			processRedirectResponse();
			break ;
		case ConfigInfo::AUTO_INDEX_ON:
			processAutoIndex();
			break ;
		case ConfigInfo::LOCATION_RESOLVED:
			setEffectiveRequestURI(request, request.address.second, _config_info.resolved_target);
			setAbsoluteFilePath(_config_info.resolved_location->_root, _config_info.resolved_file_path);
			handlerExecution(fd_table, request);
			break ;
		default:
			return ;
	}
}

void	Response::processCgiRequest(Request const & request)
{
	_is_cgi = _cgi_handler.isCgi(request);
}

void	Response::processRedirectResponse()
{
	std::string	redirect_text = _config_info.resolved_location->_return.second;
	markComplete(_config_info.resolved_location->_return.first);
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

void	Response::processAutoIndex()
{
	markComplete(StatusCode::STATUS_OK);
	if (WebservUtility::list_directory(_config_info.resolved_target, _config_info.resolved_file_path, _message_body) == ERR)
	{
		_message_body.erase();
		markComplete(StatusCode::INTERNAL_SERVER_ERROR);
	}
}

void	Response::setEffectiveRequestURI(Request const & request, int port, std::string const & resolved_target)
{
	//TODO: to pick up resolved_host (if it's default server_name, take from ConfigInfo)
	std::string resolved_host;
	if (request.header_fields.contains("host"))
	{
		resolved_host = request.header_fields.find("host")->second;
	}
	else
	{
		resolved_host = "";
	}
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

void	Response::handlerExecution(FdTable & fd_table, Request & request)
{
	// Note: if request_target is "/" OR a directory: the DEFAULT index needs to be checked
	// which could also be CGI: 'index index.html index.php index.py ...'
	if (_is_cgi)
	{
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
	if (_status_code == StatusCode::METHOD_NOT_ALLOWED)
	{
		std::string	value;
		for (method_iterator it = _config_info.resolved_location->_allowed_methods.begin(); it != _config_info.resolved_location->_allowed_methods.end(); ++it)
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
	if ( _config_info.result == ConfigInfo::AUTO_INDEX_ON)
	{
		_header_fields["Content-Type"] = "text/html";
		return ;
	}
	std::string file = _file_handler.getAbsoluteFilePath();
	if (_method == GET && !file.empty())
	{
		_header_fields["Content-Type"] = MediaType::getMediaType(file);
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
	ConfigResolver	error_page_resolver(_config_info.resolved_server);
	if (error_page_resolver.resolveErrorPage(_status_code) == OK)
	{
		if(_file_handler.redirectErrorPage(fd_table, error_page_resolver.getConfigInfo().resolved_file_path, _status_code) == OK)
		{
			return true;
		}
	}
	return false;
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
