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
	- REQUEST_COMPLETE
	- CONT_READING
	- BAD_REQUEST // Check status_code for specific error value
*/
int RequestParser::parseHeader(std::string const & request)
{
	_index = 0;

	if (WebservUtility::findLimit(request, EOHEADER, MAX_HEADER_SIZE) == std::string::npos)
	{
		return BAD_REQUEST;
	}

	resetParser();

	if (parseRequestLine(request) == ERR)
	{
		return BAD_REQUEST;
	}

	if (parseHeaderFields(request) == ERR)
	{
		return BAD_REQUEST;
	}

	parseMessageBody(request);
	return REQUEST_COMPLETE;
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
		// NginX skips all slashes
		while (s[_index] == '/')
		{
			++_index;
		}
		if (!isPchar(s[_index]))
		{
			break;
		}
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
		if (!isVchar(request[_index]))
		{
			return ERR;
		}
		skip(request, isVchar);
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

/*
Message Body parsing
*/

int RequestParser::parseMessageBody(std::string const & request)
{
	header_field_t::iterator it = _header_fields.find("content-length");
	if (it == _header_fields.end())
	{
		return OK;
	}

	std::size_t content_length = WebservUtility::strtoul(it->second);
	if (content_length == 0)
	{
		_message_body = request.substr(_index);
	}
	else
	{
		_message_body = request.substr(_index, content_length);
	}
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

enum RequestParser::MethodType RequestParser::getMethodType(std::string const & s) const
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

void RequestParser::resetParser()
{
	_header_fields.clear();
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

/*
Debugging
*/

std::string RequestParser::getMethodString() const
{
	switch (_method)
	{
		case GET:
			return "GET";
		case POST:
			return "POST";
		case DELETE:
			return "DELETE";
		case OTHER:
			break;
	}
	return "OTHER";
}

void RequestParser::print() const
{
	printf(GREEN_BOLD "-- PARSED REQUEST --" RESET_COLOR "\r\n");
	printf("%s %s HTTP/%d.%d\r\n",
		getMethodString().c_str(), getTargetResource().c_str(),
		getHttpVersion().major, getHttpVersion().minor);
	
	for (header_field_t::const_iterator it = _header_fields.begin(); it != _header_fields.end(); ++it)
	{
		printf("  %s: %s\r\n", it->first.c_str(), it->second.c_str());
	}
	printf(GREEN_BOLD "-- MESSAGE BODY --" RESET_COLOR "\r\n");
	printf("%s\r\n", _message_body.c_str());
}
