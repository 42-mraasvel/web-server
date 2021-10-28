#include "RequestParser.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include <sstream>

RequestParser::HttpVersion::HttpVersion() {}

RequestParser::HttpVersion::HttpVersion(int maj, int min)
: major(maj), minor(min) {}

RequestParser::RequestParser()
: _status_code(200), _request(NULL), _index(0) {}

RequestParser::~RequestParser() {
	while (!_requests.empty())
	{
		Request* x = _requests.front();
		_requests.pop();
		delete x;
	}
}

/*
Returns NULL if no request, otherwise returns next request in queue and removes it from the queue
Returned pointer has to be deleted by the caller

Request status:
	HEADER_COMPLETE: still reading in message-body DATA
	COMPLETE: can safely be deleted
	BAD_REQUEST: error encountered in parsing

IF HEADER_COMPLETE
	an internal pointer to the request is stored in the parser, and message-body is still appended
	if the entire message-body is read, it's status will be set to COMPLETE
*/
Request* RequestParser::getNextRequest()
{
	if (_requests.empty())
	{
		return NULL;
	}

	Request* request = _requests.front();
	_requests.pop();
	return request;
}

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
	// If there's nothing in _buffer, we can just swap
	_buffer.append(buffer);

	if (leftOverRequest())
	{
		parseMessageBody();
	}

	while (_index < _buffer.size())
	{
		if (!checkHeaderEnd())
		{
			break;
		}
		newRequest();
		if (parseHeader() == ERR)
		{
			// clearToEoHeader();
			delimitRequest(Request::BAD_REQUEST);
		}
		else
		{
			parseMessageBody();
		}
	}
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
			// Exceeded MAX_HEADER_SIZE
			// ADD BAD_REQUEST TO QUEUE
			std::cerr << "PARSER ERROR: max header size exceeded" << std::endl;
			newRequest();
			delimitRequest(Request::BAD_REQUEST);
			resetBuffer();
		}
		return false;
	}
	return true;
}

/*
Precondition: EOHEADER is in the buffer after the index
Returns ERR on syntax error
*/
int RequestParser::parseHeader()
{
	if (parseRequestLine() == ERR)
	{
		return ERR;
	}
	if (parseHeaderFields() == ERR)
	{
		return ERR;
	}
	if (parseEndLine() == ERR)
	{
		return ERR;
	}

	if (checkHeaderFields() == ERR)
	{
		return ERR;
	}
	return OK;
}

/*
1. check content-length
2. check chunked (transfer-encoding and (?)content-encoding(?)) : overwrites content-length
*/
int RequestParser::checkHeaderFields()
{
	_body_type = NOT_PRESENT;
	header_field_t::pair_type content_length = _request->header_fields.get("Content-Length");
	header_field_t::pair_type encoding = _request->header_fields.get("Transfer-Encoding");

	if (content_length.second && encoding.second)
	{
		std::cerr << "Both content-length and transfer-encoding are present" << std::endl;
		return ERR;
	}

	if (content_length.second)
	{
		return parseContentLength(content_length.first->second);
	}
	else if (encoding.second)
	{
		return parseTransferEncoding(encoding.first->second);
	}

	return OK;
}

/*
Delimits the current request and adds it to the queue

1. Check if there is a message-body (Content-Length, Transfer-Encoding)
	- We want to check both fields and test whether to dispatch or not
	- default dispatch is delimit the request with COMPLETE
2. Add the message-body to the request
3. Add request to the queue with the appropriate status based on previous step
	- Add BAD_REQUEST in case of error

Sets _index to end of _buffer or if the request is complete to the start of the next request
*/
int RequestParser::parseMessageBody()
{
	switch (_body_type)
	{
		case NOT_PRESENT:
			delimitRequest(Request::COMPLETE);
			break;
		case LENGTH:
			parseContent();
			break;
		case CHUNKED:
			parseChunked();
			break;
	}

	return OK;
}

/*
If chunked has a parsing error, then there is a bad request
*/
int RequestParser::parseChunked()
{
	if (_chunked_parser.parse(_buffer, _index, _request->message_body) == ERR)
	{
		delimitRequest(Request::BAD_REQUEST);
	}
	else if (_chunked_parser.finished())
	{
		delimitRequest(Request::COMPLETE);
	}
	return OK;
}

/*
1. Check if the rest of the body is present
2. If not present: add whatever is in the buffer and set index to end
3. If present: add everything and set Request::COMPLETE
*/
int RequestParser::parseContent()
{
	if (_remaining_content > _buffer.size() - _index)
	{
		_request->message_body.append(_buffer.substr(_index));
		_remaining_content -= _buffer.size() - _index;
		_index = _buffer.size();
		delimitRequest(Request::HEADER_COMPLETE);
	}
	else
	{
		_request->message_body.append(_buffer.substr(_index, _remaining_content));
		_index += _remaining_content;
		_remaining_content = 0;
		delimitRequest(Request::COMPLETE);
	}

	return OK;
}

int RequestParser::parseContentLength(std::string const & value)
{
	for (std::size_t i = 0; i < value.size(); ++i)
	{
		if (!isDigit(value[i]))
		{
			// invalid value
			return ERR;
		}
	}

	if (WebservUtility::strtoul(value, _remaining_content) == -1)
	{
		// Overflow
		return ERR;
	}
	_body_type = LENGTH;
	return OK;
}

int RequestParser::parseTransferEncoding(std::string const & value)
{
	if (!WebservUtility::caseInsensitiveEqual(value, "chunked"))
	{
		//TODO: status: 501: NOT IMPLEMENTED
		_request->status_code = 501;
		return ERR;
	}
	_body_type = CHUNKED;
	return OK;
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
	_request->target_resource = _buffer.substr(start, _index - start);
	start = _index;
	skipQuery();
	_request->query = _buffer.substr(start, _index - start);
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

void RequestParser::resetBuffer()
{
	_buffer.clear();
	_index = 0;
}

void RequestParser::clearToEoHeader()
{
	_index = _buffer.find(EOHEADER, _index) + 4;
	clearToIndex();
}

int RequestParser::delimitRequest(Request::RequestStatus status)
{
	//TODO: clean up function and make it more logical
	if (_request->status == Request::READING)
	{
		_requests.push(_request);
	}

	if (status == Request::BAD_REQUEST)
	{
		//TODO: set status_code to specific version
		//NOTE: the entire buffer is discarded on a bad request
		_request->status_code = 400;
		resetBuffer();
	}

	_request->status = status;
	if (status != Request::HEADER_COMPLETE)
	{
		_request = NULL;
	}
	return OK;
}
