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
#include <dirent.h>

Response::Response(Request const & request):
_method(request.method),
_request_target(request.request_target),
_status_code(0),
_config_info(request.config_info),
_status(START),
_header_part_set(false),
_encoding(UNDEFINED),
_close_connection(request.close_connection),
_is_cgi(false),
_handler(&_file_handler)
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
	if (processCgiRequest(request) == ERR)
		return ;
	switch(_config_info.result)
	{
		case ConfigInfo::REDIRECT:
			markComplete(_config_info.resolved_location->_return.first);
			break ;
		case ConfigInfo::AUTO_INDEX_ON:
			markComplete(StatusCode::STATUS_OK);
			break ;
		case ConfigInfo::LOCATION_RESOLVED:
			if (checkRequestTarget(request) == ERR)
				return ;
			setEffectiveRequestURI(request, _config_info.resolved_target);
			setAbsoluteFilePath(_config_info.resolved_location->_root, _config_info.resolved_file_path);
			// Note: if request_target is "/" OR a directory: the DEFAULT index needs to be checked
			// which could also be CGI: 'index index.html index.php index.py ...'
			if (_handler->executeRequest(fd_table, request) == ERR)
			{
				markComplete(_handler->getStatusCode());
			}
			break ;
		default:
			return ;
	}
}

int	Response::processCgiRequest(Request const & request)
{
	if(CgiHandler::isCgi(request))
	{
		_is_cgi = true;
		_handler = &_cgi_handler;
	}
	if (_method == Method::POST && !_is_cgi)
	{
		markComplete(StatusCode::METHOD_NOT_ALLOWED);
		return ERR;
	}
	return OK;
}

int	Response::checkRequestTarget(Request const & request)
{
	//TODO: discuss PATH_INFO part of CGI and where it should be resolved
	std::string file_path = _handler->resolvedRequestTarget(request);
	if (!WebservUtility::isFileExist(file_path))
	{
		markComplete(StatusCode::NOT_FOUND);
		return ERR;
	}
	DIR*	dir = opendir(file_path.c_str());
	if (dir != NULL)
	{
		markComplete(StatusCode::MOVED_PERMANENTLY);
		setEffectiveRequestURI(request, _config_info.resolved_target.append("/"));
		closedir(dir);
		return ERR;
	}
	return OK;
}

void	Response::setEffectiveRequestURI(Request const & request, std::string const & resolved_target)
{
	std::string host;
	if (request.header_fields.contains("host"))
	{
		host = request.header_fields.find("host")->second;
	}
	else
	{
		host = request.interface_addr.first;
	}
	int	port = request.interface_addr.second;

	std::string URI_scheme = "http://";
	std::string authority = host;
	if (port != DEFAULT_PORT)
	{
		authority += ":" + WebservUtility::itoa(port);
	}
	_effective_request_uri = URI_scheme + authority + resolved_target;
}

//TODO: to improve
void	Response::setAbsoluteFilePath(std::string const & root, std::string const & resolved_file_path)
{
	if (!_is_cgi)
	{
		_file_handler.setAbsoluteFilePath(resolved_file_path);
	}
}

/****************************************************/
/****** (Client::writeEvent) generate response ******/
/***************************************************/

void	Response::generateResponse()
{
	if (_encoding == CHUNKED)
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
		_status_code = _handler->getStatusCode();
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
	setContentType();
	_handler->setSpecificHeaderField(_header_fields);
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
		// TODO: to check with Maarten how CGI is set?
		_header_fields["Location"] = _request_target;
	}
	else if (StatusCode::isStatusCode3xx(_status_code))
	{
		_header_fields["Location"] = _effective_request_uri;
	}
}

void	Response::setRetryAfter()
{
	if (StatusCode::isStatusCode3xx(_status_code))
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
			if (!(!_is_cgi && *it == "POST")) // if not CGI and method is POST, by default set METHOD_NOT_ALLOWED (regardles of config file)
			{
				value.append(*it + ", ");
			}
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
	if (_encoding == CHUNKED)
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
	if (!StatusCode::isStatusCodeNoMessageBody(_status_code))
	{
		_header_fields["Content-Length"] = WebservUtility::itoa(_message_body.size());
	}
}

void	Response::setContentType()
{
	if ( _config_info.result == ConfigInfo::AUTO_INDEX_ON)
	{
		_header_fields["Content-Type"] = "text/html";
		return ;
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
	if (_status != COMPLETE)
	{
		_handler->update();
		if (_handler->isError())
		{
			markComplete(_handler->getStatusCode());
		}
	}
	setMessageBody(fd_table);
	setEncoding();
	if (_handler->isComplete())
	{
		markComplete(_handler->getStatusCode());
	}
}

void	Response::setEncoding()
{
	if (_encoding == UNDEFINED)
	{
		if ((_http_version == "HTTP/1.1" && _method == Method::GET)
			&& _message_body.size() >= CHUNK_THRESHOLD)
		{
			_encoding = CHUNKED;
		}
		else
		{
			_encoding = NOT_CHUNKED;
		}
	}
}

/*************************************************/
/****** Client::update() - set message body ******/
/*************************************************/

void	Response::setMessageBody(FdTable & fd_table)
{
	if (_status == COMPLETE && !StatusCode::isStatusCodeNoMessageBody(_status_code))
	{
		if (_config_info.result == ConfigInfo::REDIRECT)
		{
			processRedirectResponse();
		}
		else if (_config_info.result == ConfigInfo::AUTO_INDEX_ON && _status_code == StatusCode::STATUS_OK)
		{
			processAutoIndex();
		}
		else if (_message_body.empty())
		{
			if (isErrorPageRedirected(fd_table))
			{
				_status = START;
				if (_is_cgi)
				{
					_is_cgi = false;
					_handler = &_file_handler;
				}
			}
			else
			{
				setOtherErrorPage();
			}
		}
	}
	else if (_status != COMPLETE)
	{
		_handler->setMessageBody(_message_body);
	}
}

void	Response::processRedirectResponse()
{
	std::string	redirect_text = _config_info.resolved_location->_return.second;
	if (StatusCode::isStatusCode3xx(_status_code))
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
	if (WebservUtility::list_directory(_config_info.resolved_target, _config_info.resolved_file_path, _message_body) == ERR)
	{
		_message_body.erase();
		markComplete(StatusCode::INTERNAL_SERVER_ERROR);
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
	return _handler->isReadyToWrite();
}

/*******************************/
/****** utility - private ******/
/*******************************/

void	Response::markComplete(int code)
{
	_status = COMPLETE;
	_status_code = code;
}
