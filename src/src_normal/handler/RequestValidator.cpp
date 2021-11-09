#include "RequestValidator.hpp"
#include "utility/utility.hpp"
#include "utility/status_codes.hpp"
#include <algorithm>

int RequestValidator::getStatusCode() const
{
    return _status_code;
}

bool	RequestValidator::isRequestValid(Request const & request)
{
	// TODO: to change it properly with configuration
	_allowed_methods.push_back("GET");
	_allowed_methods.push_back("POST");
	_allowed_methods.push_back("DELETE");

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
	if (!findMethod(method))
	{
		_status_code = StatusCode::METHOD_NOT_ALLOWED;
		return false;		
	}
	return true;
}

bool	RequestValidator::findMethod(MethodType const method) const
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
    it = std::find(_allowed_methods.begin(), _allowed_methods.end(), method_string);
	return it != _allowed_methods.end();
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
