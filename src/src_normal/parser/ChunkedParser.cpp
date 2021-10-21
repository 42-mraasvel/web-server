#include "ChunkedParser.hpp"
#include "settings.hpp"
#include "color.hpp"
#include "utility/utility.hpp"
#include <vector>

ChunkedParser::ChunkedParser()
: _state(ChunkedParser::SIZE), _chunk_size(0), _received_cr(false) {}

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

	// reset state to _size if it's FINISHED
	int i = 0;
	while (index < buffer.size() && _state != FINISHED)
	{
		printf("ScanChar: [%d] [%c], State: [%s]\n", buffer[index],
			convertPrint(buffer[index]), getStateString(_state).c_str());
		if ((this->*state_dispatch[_state])(buffer, index, body) == ERR)
		{
			return ERR;
		}
		++i;
		if (i == 3)
		{
			break;
		}
	}
	printf("End Of ChunkedParser\n");
	// print(buffer, index);
	return OK;
}

/*
1. Append chars to leftover as long as there are hexdigits
2. if 0 HEXDIG or overflow: throw error
3. Set state to DISCARD_LINE, next state to DATA (or Trailer if chunksize is 0)
*/
int ChunkedParser::parseSize(std::string const & buffer, std::size_t & index, std::string & body)
{
	std::size_t start = index;
	skip(buffer, index, isHex);
	_leftover.append(buffer, start, index - start);

	// all possible hexdigits have been read
	if (index == buffer.size())
	{
		printf("Chunksize line is not complete, continue reading\r\n");
		return OK;
	}

	// Min of 1 hexdigit
	if (_leftover.size() == 0)
	{
		printf("No HEXDIGITS were found\r\n");
		return ERR;
	}
	printf("Chunksize is complete\r\n");

	// Overflow check
	if (WebservUtility::strtoul(_leftover, _chunk_size, 16) == ERR)
	{
		printf("Chunksize Overflowed\r\n");
		return ERR;
	}

	printf("Chunk size: %lu\n", _chunk_size);

	printf("internal buffer before clearing: [\"%s\"]\n", _leftover.c_str());
	_leftover.clear();

	// Decide next state (end of chunked is a chunksize of 0)
	printf("Next: %d %c\n", buffer[index], buffer[index]);
	if (_chunk_size == 0)
	{
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
	printf("parseChunkData: %lu\n", _chunk_size);
	if (buffer.size() - index >= _chunk_size)
	{
		printf("Buffer contains at least the entire chunk\n");
		body.append(buffer, index, _chunk_size);
		_next_state = SIZE;
		_state = ENDLINE;
		index += _chunk_size;
	}
	else
	{
		printf("Buffer contains a partial chunk\n");
		body.append(buffer, index);
		_chunk_size -= buffer.size() - index;
		index = buffer.size();
	}
	return OK;
}

int ChunkedParser::parseTrailer(std::string const & buffer, std::size_t & index, std::string & body)
{
	if (buffer.find(CRLF, index) == std::string::npos)
	{
		_buffer.append(buffer, index);
		index = buffer.size();
		return OK;
	}

	index = buffer.find(CRLF, index);
	_buffer.clear();
	// parseEndLine(buffer);
	if (buffer.compare(index, 2, CRLF) == 0)
	{
		// parseEndLine(buffer);
		_state = FINISHED;
	}
	return OK;
}

int ChunkedParser::parseEndLine(std::string const & buffer, std::size_t & index, std::string & body)
{
	if (_state == ENDLINE)
	{
		_state = SIZE;
	}
	// one byte edgecase
	_state = _next_state;
	if (buffer.size() - index == 1)
	{
		index += 1;
		if (buffer[0] == '\n' && _received_cr == true)
		{
			_received_cr = false;
			return OK;
		}
		else if (buffer[0] == '\r' && _received_cr == false)
		{
			_received_cr = true;
			return OK;
		}
		return false;
	}

	if (buffer.compare(index, 2, CRLF) != 0)
	{
		return ERR;
	}
	index += 2;
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
		printf("Found NEWLINE at start of next buffer\r\n");
		++index;
		_leftover.clear();
		_state = _next_state;
		return OK;
	}

	index = buffer.find(CRLF, index);
	if (index != std::string::npos)
	{
		printf("CRLF found, discarded rest of line\r\n");
		index += 2;
		_state = _next_state;
		return OK;
	}
	else if (buffer[buffer.size() - 1] == '\r')
	{
		printf("End of line is carriage return\r\n");
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
	&& (_leftover.size() == 0 || !(_leftover[_buffer.size() - 1] == '\r' && buffer[0] == '\n')))
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


/*
Static (non-member) Functions
*/

ssize_t ChunkedParser::send(int fd, std::string const & body)
{
	return 0;
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
