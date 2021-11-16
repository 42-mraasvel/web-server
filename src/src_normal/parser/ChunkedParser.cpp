#include "ChunkedParser.hpp"
#include "settings.hpp"
#include "color.hpp"
#include "utility/utility.hpp"
#include "ParserUtils.hpp"
#include <vector>
#include <limits>

/*
TODO: implement for HeaderFieldParser
*/
static bool IsValidChunkedField(std::string const & key,
		std::string const & value, HeaderField const & header)
{
	return true;
}

ChunkedParser::ChunkedParser()
: _state(ChunkedParser::SIZE),
_max_size(std::numeric_limits<std::size_t>::max()),
_chunk_size(0),
_header_parser(IsValidChunkedField, MAX_HEADER_SIZE) {}

ChunkedParser::StateDispatchTableType ChunkedParser::createStateDispatch()
{
	StateDispatchTableType table(FINISHED);

	table[SIZE] = &ChunkedParser::parseSize;
	table[DATA] = &ChunkedParser::parseData;
	table[TRAILER] = &ChunkedParser::parseTrailer;
	table[ENDLINE] = &ChunkedParser::parseEndLine;
	table[DISCARD_LINE] = &ChunkedParser::parseDiscardLine;
	return table;
}

// REMOVE, RM
static char convertPrint(char x)
{
	if (!isprint(x))
	{
		return '\\';
	}
	return x;
}

/*
Parses the next buffer and appends it to body
Internally keeps track of it's previous state (previous buffer inputs)
Return data: FINISHED (need bytes extracted from buffer, aka update index), CONT_READING
*/
int ChunkedParser::parse(std::string const & buffer, std::size_t & index, std::string & body)
{
	static const StateDispatchTableType state_dispatch = createStateDispatch();

	if (!isParsing())
	{
		fprintf(stderr, "ChunkedParser::parse called with invalid state\n");
		return ERR;
	}

	while (index < buffer.size() && isParsing())
	{
		if ((this->*state_dispatch[_state])(buffer, index, body) == ERR)
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

/*
1. Append chars to leftover as long as there are hexdigits
2. if 0 HEXDIG or overflow: throw error
3. Set state to DISCARD_LINE, next state to DATA (or Trailer if chunksize is 0)
*/
int ChunkedParser::parseSize(std::string const & buffer, std::size_t & index, std::string & body)
{
	std::size_t start = index;
	WebservUtility::skip(buffer, index, isHex);
	_leftover.append(buffer, start, index - start);

	if (index == buffer.size())
	{
		return OK;
	}

	if (_leftover.size() == 0)
	{
		return ERR;
	}

	if (WebservUtility::strtoul(_leftover, _chunk_size, 16) == ERR)
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
		_next_state = DATA;
	}
	_state = DISCARD_LINE;
	return OK;
}

int ChunkedParser::parseData(std::string const & buffer, std::size_t & index, std::string & body)
{
	if (buffer.size() - index >= _chunk_size)
	{
		body.append(buffer, index, _chunk_size);
		_next_state = SIZE;
		_state = ENDLINE;
		index += _chunk_size;
	}
	else
	{
		body.append(buffer, index);
		_chunk_size -= buffer.size() - index;
		index = buffer.size();
	}
	return OK;
}

/*
Store line until newline
*/
int ChunkedParser::parseTrailer(std::string const & buffer, std::size_t & index, std::string & body)
{
	std::size_t start = index;
	index = buffer.find(CRLF, index);
	if (_leftover.size() > 0 && (_leftover[_leftover.size() - 1] == '\r' && buffer[0] == '\n')) {
		// Edgecase for newline at start of next buffer
		_leftover.resize(_leftover.size() - 1); // pop_back is C++11
		index = 1;
	} else if (index != std::string::npos) {
		// CRLF is present so we append everything uo until CRLF to leftover
		_leftover.append(buffer, start, index - start);
		index += 2;
	} else {
		// No CRLF present yet
		_leftover.append(buffer, start);
		index = buffer.size();
		return OK;
	}

	if (_leftover.size() == 0) {
		_state = FINISHED;
	}
	_leftover.clear();
	return OK;
}

int ChunkedParser::parseEndLine(std::string const & buffer, std::size_t & index, std::string & body)
{
	if (buffer.compare(index, 2, CRLF) == 0 || (_leftover == "\r" && buffer[index] == '\n'))
	{
		// printf("parseEndLine: Found CRLF\r\n");
		if (_leftover.size() == 1)
		{
			index += 1;
		}
		else
		{
			index += 2;
		}
		_state = _next_state;
		_leftover.clear();
		return OK;
	}
	else if (buffer.size() - index > 1 || buffer[index] != '\r')
	{
		// printf("No ENDILNE found when expecting one\r\n");
		return ERR;
	}
	else
	{
		// printf("parseEndLine: CR at end of buffer\r\n");
		++index;
		_leftover = "\r";
	}
	return OK;
}

/*
Discard all characters that are read until the next CRLF
Only edgecase is if the end of previous buffer had a '\r' and start of next buffer '\n'
*/
int ChunkedParser::parseDiscardLine(std::string const & buffer, std::size_t & index, std::string & body)
{
	if (_leftover == "\r" && buffer[index] == '\n')
	{
		assert(index == 0);
		// printf("Found NEWLINE at start of next buffer\r\n");
		++index;
		_leftover.clear();
		_state = _next_state;
		return OK;
	}

	if (_leftover.size() > 0)
	{
		_leftover.clear();
	}

	std::size_t start = index;
	index = buffer.find(CRLF, index);
	if (index != std::string::npos)
	{
		// printf("CRLF found, discarded rest of line\r\n");
		index += 2;
		_state = _next_state;
		return OK;
	}
	else if (buffer[buffer.size() - 1] == '\r')
	{
		// printf("End of line is carriage return\r\n");
		_leftover = "\r";
	}
	return OK;
}

/*
Checks whether there's a CRLF in the buffer, or if the leftover has a '\r' at the back and buffer
has '\n' at the front
*/
bool ChunkedParser::hasCRLF(std::string const & buffer, std::size_t index)
{
	if (buffer.find(CRLF, index) == std::string::npos
	&& (_leftover.size() == 0 || !(_leftover[_leftover.size() - 1] == '\r' && buffer[0] == '\n')))
	{
		return false;
	}

	return true;
}

/*
Utility
*/

void ChunkedParser::skip(std::string const & buffer, std::size_t & index, IsFunctionType callback)
{
	while (callback(buffer[index]))
	{
		++index;
	}
}

void ChunkedParser::reset()
{
	_chunk_size = 0;
	_leftover.clear();
	_state = ChunkedParser::SIZE;
	_max_size = std::numeric_limits<std::size_t>::max();
	_header_parser.reset();
	_content_parser.reset();
}

bool ChunkedParser::finished() const
{
	return _state == FINISHED;
}

/* Debugging */

std::string ChunkedParser::getStateString(State state) const
{
	switch (state)
	{
		case SIZE:
			return "SIZE";
		case DATA:
			return "DATA";
		case TRAILER:
			return "TRAILER";
		case ENDLINE:
			return "ENDLINE";
		case DISCARD_LINE:
			return "DISCARD_LINE";
		case ERROR:
			return "ERROR";
		case COMPLETE:
			return "COMPLETE";
		case FINISHED:
			return "FINISHED";
	}
	return "";
}

void ChunkedParser::print(const std::string& buffer, std::size_t index) const
{
	printf(RED_BOLD "ChunkedParser" RESET_COLOR "\n");
	printf("State: [%s]\n", getStateString(_state).c_str());
	printf("Next State: [%s]\n", getStateString(_next_state).c_str());
	printf("Index: [%lu]\n", index);
	printf("Chunk Size: [%lu]\n", _chunk_size);
	printf("Internal Buffer: [%s]\n", _leftover.c_str());
	printf("External buffer:\n");
	printf("[%s]\n", buffer.c_str());
}
