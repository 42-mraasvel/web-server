#include "RequestParser.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include <sstream>

RequestParser::HttpVersion::HttpVersion() {}

RequestParser::HttpVersion::HttpVersion(int maj, int min)
: major(maj), minor(min) {}

RequestParser::RequestParser()
: _status_code(200), _request(NULL), _index(0) {}

RequestParser::~RequestParser()
{}

/*
1. Append buffer to internal string, check for EOHEADER
2. Check if there is a Request left over from the previous iteration(s)

Leftover Request:
	- Header is complete
	- Add message-body to request

	- GOTO Check if Request is finished

No request:
	- If no header: return and wait for the next buffer
	- InitRequest and parse Header
	- Add request to the Queue, parse message-body until you can

	- GOTO Check if Request is finished

Check if Request is finished:
	Finished Request:
		- Delimit request
		- Back to "No Request", using the rest of the buffer (if any)
	Unfinished Request:
		- Set request to READING
		- End
End:
	Remove everything from _buffer until the index

Definitions:
	Delimit a request: add it to the queue and set it's status to COMPLETE, and set Request* request to NULL

On BAD_REQUEST: discard current buffer and add BAD_REQUEST to queue (or until EOHEADER)
*/

int RequestParser::parse(std::string const & buffer)
{
	_buffer.append(buffer);

	if (leftOverRequest())
	{
		parseMessageBody2();
		// Header is already complete
		// Add message-body to request (this delimits the request)
		// if this empties the entire buffer, the while loop is never entered
	}

	while (_index < _buffer.size())
	{
		// Check buffer for EOHEADER, and if it exceeds the MAX_HEADER_SIZE
		// findLimit is not necessary because we do size-checking and it can at most be 2*MAX_SIZE
		if (!checkHeaderEnd())
		{
			break;
		}
		// Initialize new request
		newRequest();
		// Parse header into request
		if (parseHeader2() == ERR)
		{
			// Discard until the end of header and reset request???
			// clearToEoHeader();
		}
		else
		{
			// Parse body into request (this delimits the request)
			parseMessageBody2();
		}

		// Because the index is incremented during parsing, and request is delimited,
		// we can rely on the while condition to quit when appropriate
	}
	// Erase everything until index from buffer
	clearToIndex();
	return OK;
}

/*
Returns true if there is a request we're currently reading into
*/
bool RequestParser::leftOverRequest() const
{
	return _request != NULL && _request->status != Request::COMPLETE;
}

/*
Check if _buffer contains the EOHEADER (CRLF CRLF)
Adds BAD_REQUEST to queue if size exceeds limit

returns true if the header is present and everything is cool
*/
bool RequestParser::checkHeaderEnd()
{
	if (_buffer.find(EOHEADER, _index) == std::string::npos)
	{
		if (_buffer.size() - _index > MAX_HEADER_SIZE)
		{
			// ADD BAD_REQUEST TO QUEUE
			// Exceeded MAX_HEADER_SIZE
		}
		return false;
	}
	return true;
}

/*
Precondition: EOHEADER is in the buffer after the index
Returns ERR on syntax error
*/
int RequestParser::parseHeader2()
{
	if (parseRequestLine() == ERR)
	{
		return ERR;
	}
	if (parseHeaderFields() == ERR)
	{
		return ERR;
	}
	return OK;
}

/*
Delimits the current request and adds it to the queue
*/
int RequestParser::parseMessageBody2()
{
	return OK;
}

/*
Return:
	- REQUEST_COMPLETE
	- CONT_READING
	- BAD_REQUEST // Check status_code for specific error value
*/
int RequestParser::parseHeader(std::string const & request)
{
	// _index = 0;

	// if (WebservUtility::findLimit(request, EOHEADER, MAX_HEADER_SIZE) == std::string::npos)
	// {
	// 	return BAD_REQUEST;
	// }

	// resetParser();

	// if (parseRequestLine(request) == ERR)
	// {
	// 	return BAD_REQUEST;
	// }

	// if (parseHeaderFields(request) == ERR)
	// {
	// 	return BAD_REQUEST;
	// }

	// parseMessageBody(request);
	return REQUEST_COMPLETE;
}

/*
Request Line Parsing
See documentation for ABNF and details
*/
int RequestParser::parseRequestLine()
{
	if (parseMethod() != OK)
	{
		return ERR;
	}

	if (parseSpace() != OK)
	{
		return ERR;
	}

	if (parseTargetResource() != OK)
	{
		return ERR;
	}

	if (parseSpace() != OK)
	{
		return ERR;
	}

	if (parseVersion() != OK)
	{
		return ERR;
	}

	if (parseEndLine() != OK)
	{
		return ERR;
	}
	return OK;
}

int RequestParser::parseSpace()
{
	if (_buffer[_index] != ' ')
	{
		return ERR;
	}
	++_index;
	return OK;
}

int RequestParser::parseMethod()
{
	std::size_t start = _index;
	if (!isTokenChar(_buffer[_index]))
	{
		return ERR;
	}

	skip(isTokenChar);
	_request->method = getMethodType(_buffer.substr(start, _index - start));
	return OK;
}

int RequestParser::parseTargetResource()
{
	std::size_t start = _index;
	if (skipAbsolutePath() == false) {
		return ERR;
	}
	skipQuery();
	_request->target_resource = _buffer.substr(start, _index - start);
	return OK;
}

bool RequestParser::skipAbsolutePath()
{
	if (_buffer[_index] != '/')
	{
		return false;
	}

	while (_buffer[_index] == '/')
	{
		// NginX skips all slashes
		while (_buffer[_index] == '/')
		{
			++_index;
		}
		if (!isPchar(_buffer[_index]))
		{
			break;
		}
		skip(isPchar);
	}
	return true;
}

bool RequestParser::skipQuery()
{
	if (_buffer[_index] != '?') {
		return true;
	}

	skip(isQueryChar);
	if (_buffer[_index] == '#') {
		++_index;
	}
	return true;
}

int RequestParser::parseVersion()
{
	// parse prefix
	if (_buffer.compare(_index, 5, "HTTP/") != 0)
	{
		return ERR;
	}
	_index += 5;
	if (!parseMajorVersion())
	{
		return ERR;
	}
	// parse dot
	if (_buffer[_index] != '.')
	{
		return ERR;
	}
	++_index;

	if (!parseMinorVersion())
	{
		return ERR;
	}
	return OK;
}

bool RequestParser::parseMajorVersion()
{
	if (_buffer[_index] == '0' || !isDigit(_buffer[_index]))
	{
		return false;
	}
	std::size_t start = _index;
	skip(isDigit);
	_request->major_version = WebservUtility::strtol(_buffer.data() + start);
	return true;
}

bool RequestParser::parseMinorVersion()
{
	std::size_t start = _index;
	skip(isDigit);
	if (_index - start > 3 || _index - start == 0)
	{
		return false;
	}
	_request->minor_version = WebservUtility::strtol(_buffer.data() + start);
	return true;
}

/*
Header Field Parsing
	Precondition: Ends with CRLF CRLF
*/

int RequestParser::parseHeaderFields()
{
	while (_buffer.compare(_index, 2, CRLF) != 0)
	{
		std::string key, value;
		if (parseFieldName(key) == ERR)
		{
			return ERR;
		}
		if (parseColon() == ERR)
		{
			return ERR;
		}
		parseWhiteSpace();
		if (parseFieldValue(value) == ERR)
		{
			return ERR;
		}
		parseWhiteSpace();
		if (parseEndLine() == ERR)
		{
			return ERR;
		}
		_request->header_fields[key] = value;
	}
	return OK;
}

int RequestParser::parseFieldName(std::string & key)
{
	if (!isTokenChar(_buffer[_index]))
	{
		return ERR;
	}
	std::size_t start = _index;
	skip(isTokenChar);
	key = _buffer.substr(start, _index - start);
	return OK;
}

int RequestParser::parseColon()
{
	if (_buffer[_index] != ':')
	{
		return ERR;
	}
	_index += 1;
	return OK;
}

int RequestParser::parseWhiteSpace()
{
	if (!isWhiteSpace(_buffer[_index]))
	{
		return ERR;
	}
	skip(isWhiteSpace);
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
int RequestParser::parseFieldValue(std::string & value)
{
	std::size_t start = _index;
	std::size_t end = _index;
	while (_buffer.compare(_index, 2, CRLF) != 0)
	{
		if (!isVchar(_buffer[_index]))
		{
			return ERR;
		}
		skip(isVchar);
		end = _index;
		skip(isWhiteSpace);
	}
	value = _buffer.substr(start, end - start);
	return OK;
}

int RequestParser::parseEndLine()
{
	if (_buffer.compare(_index, 2, CRLF) != 0)
	{
		return ERR;
	}
	_index += 2;
	return OK;
}

/*
Message Body parsing
*/

// int RequestParser::parseMessageBody(std::string const & request)
// {
// 	_index += 2;
// 	header_field_t::iterator it = _header_fields.find("content-length");
// 	if (it == _header_fields.end())
// 	{
// 		return OK;
// 	}

// 	std::size_t content_length = WebservUtility::strtoul(it->second);
// 	if (content_length == 0)
// 	{
// 		_message_body = request.substr(_index);
// 	}
// 	else
// 	{
// 		_message_body = request.substr(_index, content_length);
// 	}
// 	return OK;
// }

/* Helper Functions */

void RequestParser::skip(IsFunctionT condition)
{
	while (condition(_buffer[_index]) && _index < _buffer.size())
	{
		++_index;
	}
}

MethodType RequestParser::getMethodType(std::string const & s) const
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

/* Request Functions */

void RequestParser::newRequest()
{
	_request = new Request;
}

void RequestParser::clearToIndex()
{
	_buffer.erase(0, _index);
	_index = 0;
}

void RequestParser::clearToEoHeader()
{
	_index = _buffer.find(EOHEADER, _index);
	clearToIndex();
}

/*
Getters
*/

MethodType RequestParser::getMethod() const
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
