#include "HttpRequestParser.hpp"
#include "settings.hpp"

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

}

void HttpRequestParser::parseHeader(std::string const & buffer,
	std::size_t & index, Request & request)
{

}

void HttpRequestParser::parseContent(std::string const & buffer,
	std::size_t & index, Request & request)
{

}

void HttpRequestParser::parseChunked(std::string const & buffer,
	std::size_t & index, Request & request)
{

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
