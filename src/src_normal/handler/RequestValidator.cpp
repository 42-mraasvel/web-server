#include "RequestValidator.hpp"
#include "utility/utility.hpp"
#include "utility/status_codes.hpp"
#include "ConfigResolver.hpp"
#include <algorithm>

int RequestValidator::getStatusCode() const
{
    return _status_code;
}

/************************************/
/******       pre config       ******/
/************************************/

bool	RequestValidator::isRequestValidPreConfig(Request const & request)
{
	return !isBadRequest(request.status, request.status_code)
			&& isHostValid(request)
			&& isConnectionValid(request)
			&& isHttpVersionValid(request.major_version)
			&& isMethodValid(request.method)
			&& isExpectationValid(request);
}

bool	RequestValidator::isBadRequest(Request::RequestStatus status, int request_code)
{
	if (status == Request::BAD_REQUEST)
	{
		_status_code = request_code;
		return true;
	}
	return false;
}

bool	RequestValidator::isHostValid(Request const & request)
{
	if (request.header_fields.contains("host"))
	{
		std::string host = request.header_fields.find("host")->second;
		std::size_t found = host.rfind(":");
		if (found != std::string::npos)
        {
			std::string	port_str = host.substr(found + 1);
			if (!port_str.size() || WebservUtility::strtol(port_str) != request.address.second)
			{
				_status_code = StatusCode::BAD_REQUEST;
				return false;
			}
		}
	}
	return true;
}

bool	RequestValidator::isConnectionValid(Request const & request)
{
	if (request.header_fields.contains("connection")
        && !WebservUtility::caseInsensitiveEqual(request.header_fields.find("connection")->second, "close")
        && !WebservUtility::caseInsensitiveEqual(request.header_fields.find("connection")->second, "keep-alive"))
        {
			_status_code = StatusCode::BAD_REQUEST;
            return false;
		}
	return true;
}

bool	RequestValidator::isHttpVersionValid(int http_major_version)
{
	if (http_major_version != 1)
	{
		_status_code = StatusCode::HTTP_VERSION_NOT_SUPPORTED;
		return false;
	}
	return true;
}

bool	RequestValidator::isMethodValid(MethodType const method)
{
	if (method == OTHER)
	{
		_status_code = StatusCode::NOT_IMPLEMENTED;
		return false;
	}
	return true;
}

bool	RequestValidator::isExpectationValid(Request const & request)
{
	if (request.header_fields.contains("expect") &&
		!WebservUtility::caseInsensitiveEqual(request.header_fields.find("expect")->second, "100-continue"))
	{
		_status_code = StatusCode::EXPECTATION_FAILED;
		return false;
	}
	return true;
}

/************************************/
/******      post config       ******/
/************************************/

bool	RequestValidator::isRequestValidPostConfig(Request const & request, ConfigResolver const & config_resolver)
{
	return isMethodAllowed(request.method, config_resolver.resolved_location->getAllowedMethods());
}

bool	RequestValidator::isMethodAllowed(MethodType const method, std::vector<std::string> const & allowed_methods)
{
	if (!findMethod(method, allowed_methods))
	{
		_status_code = StatusCode::METHOD_NOT_ALLOWED;
		return false;		
	}
	return true;
}


bool	RequestValidator::findMethod(MethodType const method, std::vector<std::string> const & allowed_methods) const
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
	std::vector<std::string>::const_iterator    it;
    it = std::find(allowed_methods.begin(), allowed_methods.end(), method_string);
	return it != allowed_methods.end();
}
