#include "RequestValidator.hpp"
#include "parser/Request.hpp"
#include "utility/utility.hpp"

int RequestValidator::getStatusCode() const
{
    return _status_code;
}

bool	RequestValidator::isRequestValid(Request const & request)
{
	return !isBadRequest(request.status, request.status_code)
			&& isConnectionValid(request)
			&& isHttpVersionValid(request.major_version)
			&& isMethodValid()
			&& isExpectationValid(request);
}

bool	RequestValidator::isBadRequest(Request::RequestStatus status, int request_code)
{
	if (status == Request::BAD_REQUEST)
	{
		_status_code = request_code;
		return true;
	}
	return flase;
}

bool	RequestValidator::isConnectionValid(Request const & request)
{
	if (request.header_fields.contains("connection")
        && !WebservUtility::caseInsensitiveEqual(request.header_fields.find("connection")->second, "close")
        && !WebservUtility::caseInsensitiveEqual(request.header_fields.find("connection")->second, "keep-alive"))
        {
			_status_code = 400; /* BAD REQUEST */
            return false;
		}
	}
	return true;
}

bool	RequestValidator::isHttpVersionValid(int http_major_version)
{
	if (http_major_version != 1)
	{
		_status_code = 505; /* HTTP VERSION NOT SUPPORTED */
		return false;
	}
	return true;
}

bool	RequestValidator::isMethodValid()
{
	if (_method == OTHER)
	{
		_status_code = 501; /* NOT IMPLEMENTED */ 
		return false;
	}
	if (!findMethod(_method))
	{
		_status_code = 405; /* METHOD NOT ALLOWED */ 
		return false;		
	}
	return true;
}

bool	RequestValidator::findMethod(MethodType const method) const
{
	// TODO: to change it properly with configuration
	_allowed_methods.push_back("GET");
	_allowed_methods.push_back("POST");
	_allowed_methods.push_back("DELETE");

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
		_status_code = 417; /* EXPECATION FAILED */ 
		return false;
	}
	return true;
}
