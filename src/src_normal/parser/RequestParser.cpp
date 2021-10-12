#include "RequestParser.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include <sstream>

RequestParser::HttpVersion::HttpVersion() {}

RequestParser::HttpVersion::HttpVersion(int maj, int min)
: major(maj), minor(min) {}

RequestParser::RequestParser()
: _status_code(200) {}

/*
Return:
	- HEADER_INCOMPLETE if more data has to be read
	- HEADER_COMPLETE if the message-body is not complete
	- MESSAGE_COMPLETE if header_fields and message-body are both present
	- BAD_REQUEST: Syntax error encountered
		- Method Not Implemented
		- Version not supported
		- ...
*/
int RequestParser::parseHeader(std::string const & request)
{
	_index = 0;

	if (WebservUtility::findLimit(request, EOHEADER, MAX_HEADER_SIZE) == std::string::npos)
	{
		return ERR;
	}

	if (parseRequestLine(request) == ERR)
	{
		return ERR;
	}

	if (parseHeaderFields(request) == ERR)
	{
		return ERR;
	}

	return OK;
}

/*
Request Line Parsing
See documentation for ABNF and details
*/
int RequestParser::parseRequestLine(std::string const & request)
{
	if (parseMethod(request) != OK)
	{
		return ERR;
	}

	if (parseSpace(request) != OK)
	{
		return ERR;
	}

	if (parseTargetResource(request) != OK)
	{
		return ERR;
	}

	if (parseSpace(request) != OK)
	{
		return ERR;
	}

	if (parseVersion(request) != OK)
	{
		return ERR;
	}

	if (parseEndLine(request) != OK)
	{
		return ERR;
	}
	return OK;
}

int RequestParser::parseSpace(std::string const & s)
{
	if (s[_index] != ' ')
	{
		return ERR;
	}
	++_index;
	return OK;
}

int RequestParser::parseMethod(std::string const & s)
{
	std::size_t start = _index;
	if (!isTokenChar(s[_index]))
	{
		return ERR;
	}

	skip(s, isTokenChar);
	_method = getMethodType(s.substr(start, _index - start));
	return OK;
}

int RequestParser::parseTargetResource(std::string const & s)
{
	std::size_t start = _index;
	if (skipAbsolutePath(s) == false) {
		return ERR;
	}
	skipQuery(s);
	_target_resource = s.substr(start, _index - start);
	return OK;
}

bool RequestParser::skipAbsolutePath(std::string const & s)
{
	if (s[_index] != '/')
	{
		return false;
	}

	while (s[_index] == '/')
	{
		++_index;
		skip(s, isPchar);
	}
	return true;
}

bool RequestParser::skipQuery(std::string const & s)
{
	if (s[_index] != '?') {
		return true;
	}

	skip(s, isQueryChar);
	if (s[_index] == '#') {
		++_index;
	}
	return true;
}

int RequestParser::parseVersion(std::string const & s)
{
	// parse prefix
	if (s.compare(_index, 5, "HTTP/") != 0)
	{
		return ERR;
	}
	_index += 5;
	if (!parseMajorVersion(s))
	{
		return ERR;
	}
	// parse dot
	if (s[_index] != '.')
	{
		return ERR;
	}
	++_index;

	if (!parseMinorVersion(s))
	{
		return ERR;
	}
	return OK;
}

bool RequestParser::parseMajorVersion(std::string const & s)
{
	if (s[_index] == '0' || !isDigit(s[_index]))
	{
		return false;
	}
	std::size_t start = _index;
	skip(s, isDigit);
	_version.major = WebservUtility::strtol(s.data() + start);
	return true;
}

bool RequestParser::parseMinorVersion(std::string const & s)
{
	std::size_t start = _index;
	skip(s, isDigit);
	if (_index - start > 3 || _index - start == 0)
	{
		return false;
	}
	_version.minor = WebservUtility::strtol(s.data() + start);
	return true;
}

/*
Header Field Parsing
	Precondition: Ends with CRLF CRLF
*/

int RequestParser::parseHeaderFields(std::string const & request)
{
	while (request.compare(_index, 2, CRLF) != 0)
	{
		std::string key, value;
		if (parseFieldName(request, key) == ERR)
		{
			return ERR;
		}
		if (parseColon(request) == ERR)
		{
			return ERR;
		}
		parseWhiteSpace(request);
		if (parseFieldValue(request, value) == ERR)
		{
			return ERR;
		}
		parseWhiteSpace(request);
		if (parseEndLine(request) == ERR)
		{
			return ERR;
		}
		_header_fields[key] = value;
	}
	return OK;
}

int RequestParser::parseFieldName(std::string const & request, std::string & key)
{
	if (!isTokenChar(request[_index]))
	{
		return ERR;
	}
	std::size_t start = _index;
	skip(request, isTokenChar);
	key = request.substr(start, _index - start);
	WebservUtility::convertToLowercase(key);
	return OK;
}

int RequestParser::parseColon(std::string const & request)
{
	if (request[_index] != ':')
	{
		return ERR;
	}
	_index += 1;
	return OK;
}

int RequestParser::parseWhiteSpace(std::string const & request)
{
	if (!isWhiteSpace(request[_index]))
	{
		return ERR;
	}
	skip(request, isWhiteSpace);
	return OK;
}

/*
field-value		=	*( field-content )
field-content	=	field-vchar [ 1*( SP / HTAB ) field-vchar ]
field-vchar		=	VCHAR / obs-text

Notes:
	- Can be empty
	- Can contain spaces/whitespace
*/
int RequestParser::parseFieldValue(std::string const & request, std::string & value)
{
	std::size_t start = _index;
	std::size_t end = _index;
	while (request.compare(_index, 2, CRLF) != 0)
	{
		skip(request, isTokenChar);
		end = _index;
		skip(request, isWhiteSpace);
	}
	value = request.substr(start, end - start);
	return OK;
}

int RequestParser::parseEndLine(std::string const & request)
{
	if (request.compare(_index, 2, CRLF) != 0)
	{
		return ERR;
	}
	_index += 2;
	return OK;
}

/* Helper Functions */

void RequestParser::skip(std::string const & s, IsFunctionT condition)
{
	while (condition(s[_index]) && _index < s.size())
	{
		++_index;
	}
}

enum RequestParser::MethodType RequestParser::getMethodType(std::string const & s)
{
	static const std::string types[] = {
		"GET",
		"POST",
		"DELETE"
	};

	for (int i = GET; i < OTHER; ++i)
	{
		if (types[i] == s)
		{
			return static_cast<MethodType>(i);
		}
	}
	return OTHER;
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
