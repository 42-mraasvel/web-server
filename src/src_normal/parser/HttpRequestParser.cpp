#include "HttpRequestParser.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "Request.hpp"


/*
TODO:
	- Check duplicate header fields (multiple content-length, multiple transfer-encoding, etc)
	- Check header fields that aren't allowed (both content-length and chunked for example)
*/
static bool isValidRequestHeader(std::string const & key,
					std::string const & value, HeaderField const & header)
{
	return true;
}

HttpRequestParser::HttpRequestParser()
: _state(PARSE_REQUEST_LINE),
_header_parser(isValidRequestHeader, MAX_HEADER_SIZE) {}

/*
1. Parse RequestLine
2. Parse Header
3. Parse Content (Chunked or Normal)
4. Set state to complete
*/

int HttpRequestParser::parse(std::string const & buffer, std::size_t & index, Request& request)
{
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
		processRequestHeader(request);
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
		request.message_body.swap(_content_parser.getContent());
		setComplete();
	}
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

/* Header Field Checking */

/*
Flow:
	1. RequestValidator
	2. CloseConnection flag
	3. ConfigResolver, set data properly into request
	4. Resolved request validation
	5. Present payload-body check
	6. CONTINUE check (only if everything else is valid and the request is not COMPLETE)
*/
int HttpRequestParser::processRequestHeader(Request & request)
{
	return checkContentType(request.header_fields);
}

int HttpRequestParser::checkContentType(HeaderField const & header)
{
	HeaderField::const_pair_type content_length = header.get("Content-Length");
	HeaderField::const_pair_type encoding = header.get("Transfer-Encoding");

	if (content_length.second && encoding.second)
	{
		return setError(StatusCode::BAD_REQUEST);
	}

	if (content_length.second)
	{
		return parseContentLength(content_length.first->second);
	}
	else if (encoding.second)
	{
		return parseTransferEncoding(encoding.first->second);
	}
	setState(HttpRequestParser::COMPLETE);
	return OK;
}

int HttpRequestParser::parseContentLength(std::string const & value)
{
	for (std::size_t i = 0; i < value.size(); ++i)
	{
		if (!isDigit(value[i]))
		{
			return setError(StatusCode::BAD_REQUEST);
		}
	}

	std::size_t content_length;
	if (WebservUtility::strtoul(value, content_length) == -1)
	{
		return setError(StatusCode::BAD_REQUEST);
	}
	_content_parser.setContentLength(content_length);
	setState(HttpRequestParser::PARSE_CONTENT);
	return OK;
}

int HttpRequestParser::parseTransferEncoding(std::string const & value)
{
	if (!WebservUtility::caseInsensitiveEqual(value, "chunked"))
	{
		return setError(StatusCode::NOT_IMPLEMENTED);
	}
	setState(HttpRequestParser::PARSE_CHUNKED);
	return OK;
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
	_chunked_content_parser.reset();
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
