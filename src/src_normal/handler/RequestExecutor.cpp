#include "RequestExecutor.hpp"
#include "settings.hpp"
#include "utility/status_codes.hpp"
#include "utility/utility.hpp"
#include "fd/FdTable.hpp"
#include "parser/Request.hpp"
#include "Response.hpp"
#include "CgiHandler.hpp"
#include "FileHandler.hpp"

#include <errno.h>
#include <dirent.h>

RequestExecutor::RequestExecutor():
_status(RequestExecutor::START),
_status_code(0)
{}

void    RequestExecutor::markStatus(RequestExecutor::Status status, int status_code)
{
	_status = status;
	_status_code = status_code;
}

void	RequestExecutor::executeRequest(FdTable & fd_table, Request & request, Response & response)
{
	if (isRequestComplete(request))
	{
		determineIsCgi(request, response);
		if (isLocationResolved(request))
		{
			if (isRequestTargetValid(response, request.request_target))
			{
				setAbsoluteFilePath(request, response);
				if (response.handler->executeRequest(fd_table, request) == ERR)
				{
					markStatus(HANDLER_ERROR, response.handler->getStatusCode());
				}
			}
			response.effective_request_uri = getEffectiveRequestURI(request);
		}
	}

	if (_status != START)
	{
		response.markSpecial(_status_code);
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
			abortProgram("RequestExecutor invalid request status!");
			return false;
	}
}

void	RequestExecutor::determineIsCgi(Request & request, Response & response)
{
	if (CgiHandler::isCgi(request))
	{
		response.setCgi();
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
			checkAutoIndexDirectory(request);
			return false ;
		case ConfigInfo::LOCATION_RESOLVED:
			return true;
		default :
			abortProgram("RequestExecutor invalid config result!");
			return false;
	}
}

void	RequestExecutor::checkAutoIndexDirectory(Request const & request)
{
	if (request.method != Method::GET)
	{
		markStatus(BAD_REQUEST, StatusCode::BAD_REQUEST);
	}
	else if (!WebservUtility::isFileExisted(request.config_info.resolved_file_path))
	{
		markStatus(TARGET_NOT_FOUND, StatusCode::NOT_FOUND);
	}
	else
	{
		markStatus(AUTO_INDEX_ON, StatusCode::STATUS_OK);
	}
}
static bool	isTargetDirectory(std::string const & target)
{
	return target[target.size() - 1] == '/';
}

bool	RequestExecutor::isRequestTargetValid(Response const & response, std::string const & request_target)
{
	if (response.method != Method::GET && !response.is_cgi && isTargetDirectory(request_target))
	{
		markStatus(BAD_REQUEST, StatusCode::BAD_REQUEST);
		return false;
	}
	if (response.method == Method::POST && !response.is_cgi)
	{
		return true;
	}
	if (!WebservUtility::isFileExisted(response.config_info.resolved_file_path))
	{
		if (errno == EACCES)
		{
			printf(">>>>>>>> file forbidden: %s\n", response.config_info.resolved_file_path.c_str());
			markStatus(TARGET_FORBIDDEN, StatusCode::FORBIDDEN);
		}
		else
		{
			markStatus(TARGET_NOT_FOUND, StatusCode::NOT_FOUND);
		}
		return false;
	}
	DIR*	dir = opendir(response.config_info.resolved_file_path.c_str());
	if (dir != NULL)
	{
		markStatus(TARGET_IS_DIRECTORY, StatusCode::MOVED_PERMANENTLY);
		closedir(dir);
		return false;
	}
	return true;
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

void	RequestExecutor::setAbsoluteFilePath(Request const & request, Response & response)
{
	if (!response.is_cgi)
	{
		response.file_handler.setAbsoluteFilePath(request);
	}
}
