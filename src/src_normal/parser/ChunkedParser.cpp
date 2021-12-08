#include "ChunkedParser.hpp"
#include "settings.hpp"
#include "color.hpp"
#include "utility/utility.hpp"
#include "ParserUtils.hpp"
#include "Request.hpp"
#include <vector>
#include <limits>

//TODO: implement for Chunked trailer fields
static bool IsValidChunkedField(std::string const & key,
		std::string const & value, HeaderField const & header)
{
	return true;
}

ChunkedParser::ChunkedParser()
: _state(ChunkedParser::SIZE),
_max_size(std::numeric_limits<std::size_t>::max()),
_chunk_size(0),
_discarded(0),
_status_code(StatusCode::BAD_REQUEST),
_header_parser(IsValidChunkedField, MAX_HEADER_SIZE) {}

ChunkedParser::StateDispatchTableType ChunkedParser::createStateDispatch()
{
	StateDispatchTableType table(COMPLETE);

	table[SIZE] = &ChunkedParser::parseSize;
	table[DATA] = &ChunkedParser::parseData;
	table[TRAILER] = &ChunkedParser::parseTrailer;
	table[ENDLINE] = &ChunkedParser::parseEndLine;
	table[DISCARD_LINE] = &ChunkedParser::parseDiscardLine;
	return table;
}

/*
Parses the next buffer and appends it to request
Internally keeps track of it's previous state (previous buffer inputs)
Return data: FINISHED (need bytes extracted from buffer, aka update index), CONT_READING
*/
int ChunkedParser::parse(std::string const & buffer, std::size_t & index, Request & request)
{
	static const StateDispatchTableType state_dispatch = createStateDispatch();

	if (!isParsing())
	{
		return ERR;
	}

	while (index < buffer.size() && isParsing())
	{
		if ((this->*state_dispatch[_state])(buffer, index, request) == ERR)
		{
			return setError();
		}
	}
	return OK;
}


bool ChunkedParser::isParsing() const
{
	return !isComplete() && !isError();
}

bool ChunkedParser::isComplete() const
{
	return _state == ChunkedParser::COMPLETE;
}

bool ChunkedParser::isError() const
{
	return _state == ChunkedParser::ERROR;
}

void ChunkedParser::setMaxSize(std::size_t max)
{
	_max_size = max;
}

int ChunkedParser::setComplete()
{
	_state = COMPLETE;
	return OK;
}

int ChunkedParser::setError()
{
	_state = ERROR;
	return ERR;
}

int ChunkedParser::setError(int code)
{
	_status_code = code;
	return setError();
}

int ChunkedParser::getStatusCode() const
{
	return _status_code;
}

/*
1. Append chars to leftover as long as there are hexdigits
2. if 0 HEXDIG or overflow: throw error
3. Set state to DISCARD_LINE, next state to DATA (or Trailer if chunksize is 0)
*/
int ChunkedParser::parseSize(std::string const & buffer, std::size_t & index, Request & request)
{
	std::size_t start = index;
	WebservUtility::skip(buffer, index, isHex);
	_leftover.append(buffer, start, index - start);

	if (index == buffer.size())
	{
		// Still potentially missing hexdigits
		return OK;
	}

	if (_leftover.size() == 0)
	{
		return ERR;
	}

	if (WebservUtility::strtoul(_leftover, _chunk_size, 16) == ERR
		|| WebservUtility::additionOverflow(_chunk_size, request.message_body.size()))
	{
		return ERR;
	}

	_leftover.clear();
	if (_chunk_size == 0)
	{
		// End of chunked
		_next_state = TRAILER;
	}
	else
	{
		// Kind of an ugly place to set contentLength inside of the ContentParser
		_content_parser.setContentLength(_chunk_size);
		_next_state = DATA;
	}
	_state = DISCARD_LINE;
	return OK;
}

int ChunkedParser::parseData(std::string const & buffer, std::size_t & index, Request & request)
{
	if (request.message_body.size() + _chunk_size > _max_size)
	{
		return setError(StatusCode::PAYLOAD_TOO_LARGE);
	}

	if (_content_parser.parse(buffer, index) == ERR)
	{
		return setError(_content_parser.getStatusCode());
	}

	if (_content_parser.isComplete())
	{
		_next_state = SIZE;
		_state = ENDLINE;
		request.message_body.append(_content_parser.getContent());
		_content_parser.reset();
	}
	return OK;
}

int ChunkedParser::addHeaderFields(Request & request)
{
	HeaderField& header= _header_parser.getHeaderField();

	for (HeaderField::iterator it = header.begin(); it != header.end(); ++it)
	{
		//TODO: error checking key-value pairs;
		//if they are allowed here and if they are already present etc
		request.header_fields[it->first] = it->second;
	}

	header.clear();
	return OK;
}

int ChunkedParser::parseTrailer(std::string const & buffer, std::size_t & index, Request & request)
{
	if (_header_parser.parse(buffer, index) == ERR)
	{
		return ERR;
	}

	if (_header_parser.isComplete())
	{
		setComplete();
		return addHeaderFields(request);
	}

	return OK;
}

int ChunkedParser::parseEndLine(std::string const & buffer, std::size_t & index, Request & request)
{
	if (buffer.compare(index, 2, CRLF) == 0 || (_leftover == "\r" && buffer[0] == '\n'))
	{
		WebservUtility::skipEndLine(buffer, index);
		_state = _next_state;
		_leftover.clear();
	}
	else if (buffer.size() - index == 1 && buffer[index] == '\r')
	{
		_leftover = "\r";
		++index;
	}
	else
	{
		return ERR;
	}
	return OK;
}

/*
Discard all characters that are read until the next CRLF
Only edgecase is if the end of previous buffer had a '\r' and start of next buffer '\n'
*/
int ChunkedParser::parseDiscardLine(std::string const & buffer, std::size_t & index, Request & request)
{
	std::size_t start = index;
	index = WebservUtility::findEndLine(_leftover, buffer, index);
	if (index == std::string::npos)
	{
		index = buffer.size();
		_discarded += index - start;
		if (_discarded > MAX_HEADER_SIZE)
		{
			return ERR;
		}
		if (buffer[buffer.size() - 1] == '\r')
		{
			_leftover = "\r";
		}
		return OK;
	}

	if (index - start > MAX_HEADER_SIZE)
	{
		return ERR;
	}

	WebservUtility::skipEndLine(buffer, index);
	_leftover.clear();
	_state = _next_state;
	_discarded = 0;
	return OK;
}

void ChunkedParser::reset()
{
	_chunk_size = 0;
	_leftover.clear();
	_state = ChunkedParser::SIZE;
	_status_code = StatusCode::BAD_REQUEST;
	_max_size = std::numeric_limits<std::size_t>::max();
	_header_parser.reset();
	_content_parser.reset();
}
