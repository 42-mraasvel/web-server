#include "RequestParser.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include <sstream>

RequestParser::HttpVersion::HttpVersion() {}

RequestParser::HttpVersion::HttpVersion(int maj, int min)
: major(maj), minor(min) {}

//TODO handle errors
/*
Return:
	- HEADER_INCOMPLETE if more data has to be read
	- HEADER_COMPLETE if the message-body is not complete
	- MESSAGE_COMPLETE if header_fields and message-body are both present
	- BAD_REQUEST: Syntax error encountered
*/
int RequestParser::parseHeader(std::string const & request)
{
	if (request.find(EOHEADER) == std::string::npos)
	{
		return INCOMPLETE;
	}

	if (parseRequestLine(request, REQUEST_LINE_MAX_SIZE) == ERR)
	{
		return BAD_REQUEST;
	}

/*
Hardcoded example
*/
	_method = GET;
	_target_resource = "/";
	_version = HttpVersion(1, 1);
	// _header_fields["content-length"] = "12";
	// _message_body = "Hello, World!";
	return REQUEST_COMPLETE;
}

/*
Request Line Parsing
See documentation for ABNF and details
*/
int RequestParser::parseRequestLine(std::string const & request, std::size_t max_size)
{
	if (request[0] == ' ')
	{
		return ERR;
	}
	_index = request.find(CRLF);
	if (_index > max_size)
	{
		return ERR;
	}

	std::vector<std::string> result = WebservUtility::splitString(request.substr(0, _index), ' ');
	if (result.size() != 3)
	{
		return ERR;
	}
	std::cout << "Requestline:" << std::endl;
	for (std::size_t i = 0; i < result.size(); ++i) {
		std::cout << result[i] << std::endl;
	}
	if (parseMethod(result[0]) == ERR
	|| parseTargetResource(result[1]) == ERR
	|| parseVersion(result[2]) == ERR)
	{
		return ERR;
	}
	return OK;
}

int RequestParser::parseMethod(std::string const & s)
{
	static const std::string methods[] = {
		"GET",
		"POST",
		"DELETE"
	};

	for (int i = GET; i != OTHER; ++i)
	{
		if (s == methods[i])
		{
			_method = static_cast<MethodType> (i);
			return OK;
		}
	}
	_method = OTHER;
	return OK;
}

int RequestParser::parseTargetResource(std::string const & s)
{
	_target_resource = s;
	return OK;
}

int RequestParser::parseVersion(std::string const & s)
{
	// Min: HTTP/[DIGIT].[DIGIT] = 8 characters
	if (s.find("HTTP/") != 0 || s.size() < 8)
	{
		return ERR;
	}

	std::size_t index = 5;
	std::size_t end = index;
	while (end < s.size() && isdigit(s[end]))
	{
		++end;
	}
	if (s[index] == '0' || end == s.size() || end - index > 3 || s[end] != '.')
	{
		return ERR;
	}
	//TODO: overflow check for version
	end += 1;
	index = end;
	while (end < s.size() && isdigit(s[end]))
	{
		++end;
	}
	if (end != s.size() || end - index > 3)
	{
		return ERR;
	}
	std::stringstream ss(s.substr(5));
	char dot;
	ss >> _version.major >> dot >> _version.minor;
	return OK;
}

/*
Getters
*/

enum RequestParser::MethodType RequestParser::getMethod() const
{
	return _method;
}

const std::string& RequestParser::getTargetResource() const
{
	return _target_resource;
}

RequestParser::HttpVersion RequestParser::getHttpVersion() const
{
	return _version;
}

RequestParser::header_field_t& RequestParser::getHeaderFields()
{
	return _header_fields;
}

const std::string& RequestParser::getMessageBody() const
{
	return _message_body;
}
