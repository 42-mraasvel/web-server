#include "RequestExecutor.hpp"
#include "settings.hpp"
#include "utility/status_codes.hpp"
#include "utility/utility.hpp"
#include "fd/FdTable.hpp"
#include "parser/Request.hpp"
#include "ResponseInfo.hpp"
#include "CgiHandler.hpp"
#include "FileHandler.hpp"

#include <dirent.h>

RequestExecutor::RequestExecutor():
_status_code(0),
_status(RequestExecutor::NOT_COMPLETE)
{}

void    RequestExecutor::markStatus(RequestExecutor::Status status, int status_code)
{
	_status = status;
	_status_code = status_code;
}

void	RequestExecutor::executeRequest(FdTable & fd_table, Request & request, ResponseInfo & response)
{
	if (isRequestComplete(request))
	{
		response.is_cgi = CgiHandler::isCgi(request);
		if (isLocationResolved(request))
		{
			if (isRequestTargetValid(request.config_info.resolved_file_path))
			{
				if (response.handler->executeRequest(fd_table, request) == ERR)
				{
					markStatus(HANDLER_ERROR, response.handler->getStatusCode());
				}
			}
			response.effective_request_uri = getEffectiveRequestURI(request);
			setAbsoluteFilePath(request, response);
		}
	}

	if (_status != NOT_COMPLETE)
	{
		response.setStatus(ResponseInfo::COMPLETE, _status_code);
	}
}

bool	RequestExecutor::isRequestComplete(Request const & request)
{
	switch(request.status)
	{
		case Request::BAD_REQUEST:
			markStatus(BAD_REQUEST, request.status_code);
			return false;
		case Request::EXPECT:
			markStatus(CONTINUE, StatusCode::CONTINUE);
			return false;
		case Request::COMPLETE:
			return true;
		default :
			//assert(); //TODO: check with maarten how to do;
			return true;
	}
}

bool	RequestExecutor::isLocationResolved(Request const & request)
{
	switch(request.config_info.result)
	{
		case ConfigInfo::REDIRECT:
			markStatus(REDIRECT, request.config_info.resolved_location->_return.first);
			return false ;
		case ConfigInfo::AUTO_INDEX_ON:
			markStatus(AUTO_INDEX_ON, StatusCode::STATUS_OK);
			return false ;
		case ConfigInfo::LOCATION_RESOLVED:
			return true;
		default :
			//assert(); //TODO: check with maarten how to do;
			return true;
	}
}

bool	RequestExecutor::isRequestTargetValid(std::string const & target)
{
	if (!WebservUtility::isFileExist(target))
	{
		markStatus(TARGET_NOT_FOUND, StatusCode::NOT_FOUND);
		return ERR;
	}
	DIR*	dir = opendir(target.c_str());
	if (dir != NULL)
	{
		markStatus(TARGET_IS_DIRECTORY, StatusCode::MOVED_PERMANENTLY);
		closedir(dir);
		return ERR;
	}
	return OK;
}

std::string	RequestExecutor::getEffectiveRequestURI(Request const & request)
{
	std::string URI_scheme = "http://";

	std::string authority;
	if (request.header_fields.contains("host"))
	{
		authority = WebservUtility::trimPort(request.header_fields.find("host")->second);
	}
	else
	{
		authority = request.interface_addr.first;
	}
	int	port = request.interface_addr.second;
	if (port != DEFAULT_PORT)
	{
		authority += ":" + WebservUtility::itoa(port);
	}
	
	std::string	resolved_target = request.config_info.resolved_target;
	if (_status == TARGET_IS_DIRECTORY)
	{
		resolved_target.append("/");
	}

	return URI_scheme + authority + resolved_target;
}

void	RequestExecutor::setAbsoluteFilePath(Request const & request, ResponseInfo & response)
{
	if (!response.is_cgi)
	{
		response.file_handler.setAbsoluteFilePath(request.config_info.resolved_file_path);
	}
}
