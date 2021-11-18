#include "RequestValidator.hpp"
#include "utility/utility.hpp"
#include "utility/status_codes.hpp"
#include "settings.hpp"
#include "ConfigResolver.hpp"
#include <algorithm>

RequestValidator::RequestValidator()
: _close_connection(false), _continue(false) {}

/****************************************/
/******      basic interface       ******/
/****************************************/

int RequestValidator::getStatusCode() const
{
    return _status_code;
}

bool RequestValidator::shouldCloseConnection() const
{
	return _close_connection;
}

bool	RequestValidator::shouldSendContinue() const
{
	return _continue;
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
			&& isTransferEncodingValid(request.header_fields)
			&& isContentCodingValid(request.header_fields)
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
	HeaderField::const_pair_type host = request.header_fields.get("host");
	if (host.second)
	{
		std::string value = host.first->second;
		if (value.empty())
		{
			return false;
		}
		std::size_t found = value.rfind(":");
		if (found != std::string::npos)
		{
			std::string port_str = value.substr(found + 1);
			if (!port_str.size() || WebservUtility::strtol(port_str) != request.address.second)
			{
				generalError("%s\n", _FUNC_ERR("port number mismatch").c_str());
				_status_code = StatusCode::BAD_REQUEST;
				return false;
			}
		}
	}
	else if (request.minor_version >= 1)
	{
		generalError("%s: Version: %d\n", _FUNC_ERR("HOST REQUIRED").c_str(), request.minor_version);
		//TODO: should the connection actually be closed in this case?
		_close_connection = true;
		_status_code = StatusCode::BAD_REQUEST;
		return false;
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

bool	RequestValidator::isTransferEncodingValid(const HeaderField & header)
{
	HeaderField::const_pair_type transfer_encoding = header.get("Transfer-Encoding");
	if (transfer_encoding.second)
	{
		if (!WebservUtility::caseInsensitiveEqual(transfer_encoding.first->second, "chunked"))
		{
			generalError("%s %s\n", _FUNC_ERR("Unsupported TE:").c_str(), transfer_encoding.first->second.c_str());
			_status_code = StatusCode::NOT_IMPLEMENTED;
			_close_connection = true;
			return false;
		}
	}
	return true;
}

bool	RequestValidator::isContentCodingValid(const HeaderField & header)
{
	HeaderField::const_pair_type content_coding = header.get("Content-Coding");
	if (content_coding.second)
	{
		generalError("%s\n", _FUNC_ERR("Content Coding Present").c_str());
		if (content_coding.first->second.size() == 0)
		{
			_status_code = StatusCode::BAD_REQUEST;
			return false;
		}
		_status_code = StatusCode::UNSUPPORTED_MEDIA_TYPE;
		return false;
	}
	return true;
}

bool	RequestValidator::isExpectationValid(Request const & request)
{
	HeaderField::const_pair_type expect = request.header_fields.get("expect");
	if (!expect.second)
	{
		return true;
	}

	if (!WebservUtility::caseInsensitiveEqual(expect.first->second, "100-continue"))
	{
		_status_code = StatusCode::EXPECTATION_FAILED;
		return false;
	}
	_continue = true;
	return true;
}

/************************************/
/******      post config       ******/
/************************************/

bool	RequestValidator::isRequestValidPostConfig(Request const & request)
{
	return isMethodAllowed(request.method, request.config_info.resolved_location->_allowed_methods);
}

//TODO: delete this version
bool	RequestValidator::isRequestValidPostConfig(Request const & request, ConfigInfo const & config_info)
{
	return isMethodAllowed(request.method, config_info.resolved_location->_allowed_methods);
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
