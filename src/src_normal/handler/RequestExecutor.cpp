#include "RequestExecutor.hpp"
#include "settings.hpp"
#include "utility/status_codes.hpp"
#include "utility/utility.hpp"
#include "parser/Request.hpp"
#include "ResponseInfo.hpp"

RequestExecutor::RequestExecutor():
_status_code(0),
_status(OK),
_is_cgi(false)
{}


void	RequestExecutor::executeRequest(FdTable & fd_table, Request & request, ResponseInfo & response)
{
	if (dispatchRequest(fd_table, request) == OK)
	{
		_is_cgi = CgiHandler::isCgi(request);
		if (dispatchConfigResult(fd_table, request) == OK)
		{
			if (checkRequestTarget(request) == OK)
			{
				setEffectiveRequestURI(request, request.config_info.resolved_target);
				setAbsoluteFilePath(request.config_info.resolved_file_path);
				if (_handler->executeRequest(fd_table, request) == ERR)
				{
					markStatus(HANDLER_ERROR, _handler->getStatusCode());
				}
			}
		}
	}
}

int RequestExecutor::dispatchRequest(Request const & request)
{
	switch(request.status)
	{
		case Request::BAD_REQUEST:
			markStatus(BAD_REQUEST, request.status_code);
			return ERR;
		case Request::EXPECT:
			markStatus(100_CONTINUE, StatusCode::CONTINUE);
			return ERR;
		case Request::COMPLETE:
			return OK;
	}
}

int	RequestExecutor::dispatchConfigResult(Request const & request)
{
	switch(request.config_info.result)
	{
		case ConfigInfo::REDIRECT:
			markStatus(REDIRECT, request.config_info.resolved_location->_return.first);
			return ERR ;
		case ConfigInfo::AUTO_INDEX_ON:
			markStatus(AUTO_INDEX_ON, StatusCode::STATUS_OK);
			return ERR ;
		case ConfigInfo::LOCATION_RESOLVED:
			return OK;
}


int	RequestExecutor::checkRequestTarget(Request const & request)
{
	if (!WebservUtility::isFileExist(request.config_info.resolved_file_path))
	{
		markStatus(TARGET_ERROR, StatusCode::NOT_FOUND);
		return ERR;
	}
	DIR*	dir = opendir(request.config_info.resolved_file_path.c_str());
	if (dir != NULL)
	{
		markStatus(TARGET_ERROR, StatusCode::MOVED_PERMANENTLY);
		std::string new_target = request.config_info.resolved_target + "/";
		setEffectiveRequestURI(request, new_target);
		closedir(dir);
		return ERR;
	}
	return OK;
}

void	RequestExecutor::setEffectiveRequestURI(Request const & request, std::string const & resolved_target)
{
	std::string host;
	if (request.header_fields.contains("host"))
	{
		host = WebservUtility::trimPort(request.header_fields.find("host")->second);
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
	if (_status_code == StatusCode::MOVED_PERMANENTLY)
	{
		
	}
	_info.effective_request_uri = URI_scheme + authority + resolved_target;
}

void	RequestExecutor::setAbsoluteFilePath(std::string const & resolved_file_path)
{
	if (!_info.is_cgi)
	{
		_file_handler.setAbsoluteFilePath(resolved_file_path);
	}
}

void    RequestExecutor::markStatus(RequestExecutor::Status status, int status_code)
{
	_status = status;
	_status_code = status_code;
}
