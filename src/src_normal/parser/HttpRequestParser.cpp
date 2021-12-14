#include "HttpRequestParser.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "Request.hpp"
#include <iostream>

static bool validHostHeaderValue(const std::string& value)
{
	std::size_t pos = value.find(':');
	if (pos == std::string::npos)
	{
		return true;
	}
	++pos;
	while (pos < value.size())
	{
		if (!isdigit(value[pos]))
		{
			return false;
		}
		++pos;
	}
	return true;
}

/*
This is the validator called during HeaderFieldParsing
Used to check duplicate header-fields
[RFC7230] Section 3.2.2.
*/
static bool isValidRequestHeader(std::string const &key,
								 std::string const &value, HeaderField const &header)
{
	HeaderField::const_pair_type field = header.get(key);
	if (field.second && !WebservUtility::caseInsensitiveEqual(key, "Set-Cookie"))
	{
		std::cerr << (_FUNC_ERR("Duplicate Field")) << ": " << key << std::endl;
		return false;
	}
	if (WebservUtility::caseInsensitiveEqual(key, "Host"))
	{
		return validHostHeaderValue(value);
	}
	return true;
}

HttpRequestParser::HttpRequestParser(MapType const * config_map)
: _state(PARSE_REQUEST_LINE),
_header_parser(isValidRequestHeader, MAX_HEADER_SIZE),
_header_processor(config_map) {}

/*
1. Parse RequestLine
2. Parse Header
3. Parse Content (Chunked or Normal)
4. Set state to complete
*/

int HttpRequestParser::parse(std::string const &buffer, std::size_t &index, Request &request)
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
				_header_processor.processError(request);
				return ERR;
			case COMPLETE:
				return OK;
		}
	}
	if (_state == ERROR)
	{
		_header_processor.processError(request);
		return ERR;
	}
	return OK;
}

/* Main Parsing Logic */

void HttpRequestParser::parseRequestLine(std::string const &buffer,
										 std::size_t &index, Request &request)
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

void HttpRequestParser::parseHeader(std::string const &buffer,
									std::size_t &index, Request &request)
{
	if (_header_parser.parse(buffer, index) == ERR)
	{
		headerErrorCheckCloseConnection(request);
		setError(_header_parser.getStatusCode());
	}
	else if (_header_parser.isComplete())
	{
		request.header_fields.swap(_header_parser.getHeaderField());
		processRequestHeader(request);
	}
}

void HttpRequestParser::headerErrorCheckCloseConnection(Request & request)
{
	const std::pair<std::string, std::string>& failed = _header_parser.getFailedPair();

	if (WebservUtility::caseInsensitiveEqual(failed.first, "Content-Length")
		&& _header_parser.getHeaderField().contains(failed.first))
	{
		request.close_connection = true;
	}
}

void HttpRequestParser::parseContent(std::string const &buffer,
									 std::size_t &index, Request &request)
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

void HttpRequestParser::parseChunked(std::string const &buffer,
									 std::size_t &index, Request &request)
{
	if (_chunked_content_parser.parse(buffer, index, request) == ERR)
	{
		request.close_connection = true;
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
	1. Process request header (Checks errors, resolves configuration)
	2. Present payload-body check
*/
int HttpRequestParser::processRequestHeader(Request &request)
{
	if (_header_processor.process(request) == ERR)
	{
		return setError(_header_processor.getStatusCode());
	}

	_content_parser.setMaxSize(request.config_info.resolved_server->_client_body_size);

	if (checkContentType(request.header_fields) == ERR)
	{
		request.close_connection = true;
		return ERR;
	}
	return OK;
}

int HttpRequestParser::checkContentType(HeaderField const &header)
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

int HttpRequestParser::parseContentLength(std::string const &value)
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

	if (content_length > _content_parser.getMaxSize())
	{
		return setError(StatusCode::PAYLOAD_TOO_LARGE);
	}

	_content_parser.setContentLength(content_length);
	if (content_length == 0)
	{
		setState(HttpRequestParser::COMPLETE);
	}
	else
	{
		setState(HttpRequestParser::PARSE_CONTENT);
	}
	return OK;
}

int HttpRequestParser::parseTransferEncoding(std::string const &value)
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
