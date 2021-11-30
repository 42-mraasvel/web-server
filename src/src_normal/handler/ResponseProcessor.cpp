#include "ResponseProcessor.hpp"
#include "settings.hpp"
#include "CgiHandler.hpp"
#include "parser/Request.hpp"
#include "fd/FdTable.hpp"
#include "utility/utility.hpp"
#include "parser/Request.hpp"
#include "fd/FdTable.hpp"
#include <dirent.h>

ResponseProcessor::ResponseProcessor(Request const & request):
_info(request),
_handler(&_file_handler)
{}


/*********************************************/
/****** (Client::update) execute request *****/
/*********************************************/

void	ResponseProcessor::executeRequest(FdTable & fd_table, Request & request)
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
			executeCompleteRequest(fd_table, request);
			break ;
		default:
			return ;
	}
}

void	ResponseProcessor::executeCompleteRequest(FdTable & fd_table, Request & request)
{
	if(CgiHandler::isCgi(request))
	{
		setCgi();
	}
	dispatchConfigResult(fd_table, request);
}

void	ResponseProcessor::dispatchConfigResult(FdTable & fd_table, Request & request)
{
	switch(request.config_info.result)
	{
		case ConfigInfo::REDIRECT:
			markComplete(request.config_info.resolved_location->_return.first);
			break ;
		case ConfigInfo::AUTO_INDEX_ON:
			markComplete(StatusCode::STATUS_OK);
			break ;
		case ConfigInfo::LOCATION_RESOLVED:
			if (checkRequestTarget(request) == ERR)
				return ;
			if (_handler->executeRequest(fd_table, request) == ERR)
			{
				markComplete(_handler->getStatusCode());
			}
			break ;
		default:
			return ;
	}
}

int	ResponseProcessor::checkRequestTarget(Request const & request)
{
	if (!WebservUtility::isFileExist(request.config_info.resolved_file_path))
	{
		markComplete(StatusCode::NOT_FOUND);
		return ERR;
	}
	DIR*	dir = opendir(request.config_info.resolved_file_path.c_str());
	if (dir != NULL)
	{
		markComplete(StatusCode::MOVED_PERMANENTLY);
		std::string new_target = request.config_info.resolved_target + "/";
		setEffectiveRequestURI(request, new_target);
		closedir(dir);
		return ERR;
	}
	setEffectiveRequestURI(request, request.config_info.resolved_target);
	setAbsoluteFilePath(request.config_info.resolved_file_path);
	return OK;
}

void	ResponseProcessor::setEffectiveRequestURI(Request const & request, std::string const & resolved_target)
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
	_info.effective_request_uri = URI_scheme + authority + resolved_target;
}

void	ResponseProcessor::setAbsoluteFilePath(std::string const & resolved_file_path)
{
	if (!_info.is_cgi)
	{
		_file_handler.setAbsoluteFilePath(resolved_file_path);
	}
}

/************************************************/
/****** Client::update() - updateResponse ******/
/************************************************/

void	ResponseProcessor::updateResponse(FdTable & fd_table, std::string & message_body)
{
	if (_info.status != ResponseInfo::COMPLETE)
	{
		try
		{
			_handler->update();
		}
		catch (std::exception const & e)
		{
			fprintf(stderr, "%sUPDATE RESPONSE EXCEPTION%s: [%s]\n",
				RED_BOLD, RESET_COLOR, e.what());
			_handler->exceptionEvent();
		}
		if (_handler->isError())
		{
			markComplete(_handler->getStatusCode());
		}
	}
	setMessageBody(fd_table, message_body);
	if (_handler->isComplete())
	{
		markComplete(_handler->getStatusCode());
	}
	setEncoding(message_body);
}

void	ResponseProcessor::setEncoding(std::string const & message_body)
{
	if (_info.encoding == ResponseInfo::UNDEFINED)
	{
		if (_info.status != ResponseInfo::COMPLETE)
		{
			if (isReadyToBeChunked(message_body))
			{
				_info.encoding = ResponseInfo::CHUNKED;
			}
		}
		else
		{
			if (isReadyToBeChunked(message_body))
			{
				_info.encoding = ResponseInfo::CHUNKED;
			}
			else
			{
				_info.encoding = ResponseInfo::NOT_CHUNKED;
			}
		}
	}
}

bool	ResponseProcessor::isReadyToBeChunked(std::string const & message_body) const
{
	return _info.http_version == "HTTP/1.1"
				&& message_body.size() >= CHUNK_THRESHOLD;
}

/*************************************************/
/****** Client::update() - set message body ******/
/*************************************************/

void	ResponseProcessor::setMessageBody(FdTable & fd_table, std::string & message_body)
{
	if (_info.status == ResponseInfo::COMPLETE && !StatusCode::isStatusCodeNoMessageBody(_info.status_code))
	{
		if (_info.config_info.result == ConfigInfo::REDIRECT)
		{
			processRedirectResponse(message_body);
		}
		else if (_info.config_info.result == ConfigInfo::AUTO_INDEX_ON && _info.status_code == StatusCode::STATUS_OK)
		{
			processAutoIndex(message_body);
		}
		else if (message_body.empty())
		{
			if (!_info.error_page_attempted && !isErrorPageRedirected(fd_table))
			{
				setOtherErrorPage(message_body);
			}
		}
	}
	else if (_info.status != ResponseInfo::COMPLETE)
	{
		_handler->setMessageBody(message_body);
	}
}

void	ResponseProcessor::processRedirectResponse(std::string & message_body)
{
	std::string	redirect_text = _info.config_info.resolved_location->_return.second;
	if (StatusCode::isStatusCode3xx(_info.status_code))
	{
		// TODO: to check if the redirect_text needs to be absolute form??
		_info.effective_request_uri = redirect_text;
		message_body = "Redirect to " + redirect_text + "\n";
	}
	else
	{
		message_body = redirect_text;
	}
}

void	ResponseProcessor::processAutoIndex(std::string & message_body)
{
	if (WebservUtility::list_directory(_info.config_info.resolved_target, _info.config_info.resolved_file_path, message_body) == ERR)
	{
		message_body.erase();
		markComplete(StatusCode::INTERNAL_SERVER_ERROR);
	}
}

bool	ResponseProcessor::isErrorPageRedirected(FdTable & fd_table)
{
	std::string file_path;
	ConfigResolver	error_page_resolver(_info.config_info.resolved_server);
	if (error_page_resolver.resolveErrorPage(_info.status_code) == OK)
	{
		if (_file_handler.redirectErrorPage(fd_table, error_page_resolver.getConfigInfo().resolved_file_path, _info.status_code) == OK)
		{
			_info.status = ResponseInfo::START;
			_info.encoding = ResponseInfo::UNDEFINED;
			_info.error_page_attempted = true;
			unsetCgi();
			return true;
		}
	}
	return false;
}

void	ResponseProcessor::setOtherErrorPage(std::string & message_body)
{
	message_body = WebservUtility::itoa(_info.status_code) + " "
					+ StatusCode::getStatusMessage(_info.status_code) + "\n";
}

/*****************************/
/****** utility - public *****/
/*****************************/

bool	ResponseProcessor::isChunked() const
{
	return _info.encoding == ResponseInfo::CHUNKED;
}

ResponseInfo	ResponseProcessor::getInfo() const
{
	return _info;
}

iHandler*		ResponseProcessor::getHandler() const
{
	return _handler;
}

/******************************/
/****** utility - private *****/
/******************************/

void	ResponseProcessor::markComplete(int code)
{
	_info.status = ResponseInfo::COMPLETE;
	_info.status_code = code;
}

void	ResponseProcessor::setCgi()
{
	_info.is_cgi = true;
	_handler = &_cgi_handler;
}

void	ResponseProcessor::unsetCgi()
{
	_info.is_cgi = false;
	_handler = &_file_handler;
}
