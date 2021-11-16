#include "HttpRequestParser.hpp"
#include "settings.hpp"
#include "Request.hpp"

/*
1. Parse RequestLine
2. Parse Header
3. Parse Content (Chunked or Normal)
4. Set state to complete
*/

static bool isValidRequestHeader(std::string const & key,
					std::string const & value, HeaderField const & header)
{
	// TODO: implement
	return true;
}

HttpRequestParser::HttpRequestParser()
: _state(PARSE_REQUEST_LINE),
_header_parser(isValidRequestHeader, MAX_HEADER_SIZE) {}

int HttpRequestParser::parse(std::string const & buffer, Request& request)
{
	std::size_t index = 0;
	while (index < buffer.size())
	{
		switch (_state)
		{
			case PARSE_REQUEST_LINE:
				parseRequestLine(buffer, index, request);
				break;
			case PARSE_HEADER:
				parseHeader(buffer, index, request);
				break;
			case PARSE_CONTENT:
				parseContent(buffer, index, request);
				break;
			case PARSE_CHUNKED:
				parseChunked(buffer, index, request);
				break;
			case ERROR:
				return ERR;
			case COMPLETE:
				return OK;
		}
	}
	return OK;
}

/* Main Parsing Logic */

void HttpRequestParser::parseRequestLine(std::string const & buffer,
	std::size_t & index, Request & request)
{
	if (_request_line_parser.parse(buffer, index, request) == ERR)
	{
		setError(_request_line_parser.getStatusCode());
	}
	else if (_request_line_parser.isComplete())
	{
		setState(HttpRequestParser::PARSE_HEADER);
	}
}

void HttpRequestParser::parseHeader(std::string const & buffer,
	std::size_t & index, Request & request)
{
	if (_header_parser.parse(buffer, index) == ERR)
	{
		setError(_header_parser.getStatusCode());
	}
	else if (_header_parser.isComplete())
	{
		request.header_fields.swap(_header_parser.getHeaderField());
		if (checkHeaderFields(request.header_fields) == ERR)
		{
			setError(StatusCode::BAD_REQUEST);
		}
	}
}

void HttpRequestParser::parseContent(std::string const & buffer,
	std::size_t & index, Request & request)
{
	if (_content_parser.parse(buffer, index) == ERR)
	{
		setError(_content_parser.getStatusCode());
	}
	else if (_content_parser.isComplete())
	{
		setComplete();
	}
}

/*
TODO:
	- Check headerFields if contentParsing is necessary
	- Update contentLength in contentParser
	- Set to Chunked if chunked
	- Error check HeaderFields
	- Potential request appending for 100 continue
	- Configuration resolution
*/
int HttpRequestParser::checkHeaderFields(HeaderField const & request)
{
	setState(HttpRequestParser::PARSE_CONTENT);
	return OK;
}

void HttpRequestParser::parseChunked(std::string const & buffer,
	std::size_t & index, Request & request)
{
	if (_chunked_content_parser.parse(buffer, index, request) == ERR)
	{
		setError(_chunked_content_parser.getStatusCode());
	}
	else if (_chunked_content_parser.isComplete())
	{
		setComplete();
	}
}

/* Other functions, general interface */

int HttpRequestParser::getStatusCode() const
{
	return _status_code;
}

bool HttpRequestParser::isError() const
{
	return _state == HttpRequestParser::ERROR;
}

bool HttpRequestParser::isComplete() const
{
	return _state == HttpRequestParser::COMPLETE;
}

void HttpRequestParser::reset()
{
	_state = PARSE_REQUEST_LINE;
	_request_line_parser.reset();
	_header_parser.reset();
	_content_parser.reset();
	// TODO: set chunked
	// _chunked_content_parser.reset();
}

int HttpRequestParser::setComplete()
{
	_state = HttpRequestParser::COMPLETE;
	return OK;
}

int HttpRequestParser::setError(int code)
{
	_state = HttpRequestParser::ERROR;
	_status_code = code;
	return ERR;
}

void HttpRequestParser::setState(State new_state)
{
	_state = new_state;
}
