#include "ChunkedParser.hpp"
#include "settings.hpp"
#include "color.hpp"
#include "utility/utility.hpp"

ChunkedParser::ChunkedParser()
: _state(ChunkedParser::SIZE), _chunk_size(0), _received_cr(false) {}

/*
Parses the next buffer and appends it to body
Internally keeps track of it's previous state (previous buffer inputs)
Return data: FINISHED (need bytes extracted from buffer, aka update index), CONT_READING
*/
int ChunkedParser::parse(std::string const & buffer, std::size_t & index, std::string & body)
{
	_index = index;
	while (_index < buffer.size())
	{
		switch (_state)
		{
			case SIZE:
				if (parseChunkSize(buffer) == ERR)
				{
					return ERR;
				}
				break;
			case DATA:
				parseChunkData(buffer, body);
				break;
			case TRAILER:
				parseTrailer(buffer);
				break;
			case ENDLINE:
				parseEndLine(buffer);
				break;
			case DISCARD_LINE:
				break;
			case FINISHED:
				printf("Finished\n");
				index = _index;
				print(buffer);
				return OK;
		}
	}

	printf("CONT_READING?\n");
	print(buffer);
	index = _index;
	return OK;
}

int ChunkedParser::parseChunkSize(std::string const & buffer)
{
	if (!hasCRLF(buffer))
	{
		printf("Chunksize line is not complete\n");
		_buffer.append(buffer, _index);
		_index = buffer.size();
		return OK;
	}

	printf("Chunksize line is complete\n");


	std::size_t start = _index;
	_index = buffer.find(CRLF, _index);
	_buffer.append(buffer.substr(start, _index - start));
	if (!isHex(_buffer[0]))
	{
		printf("Invalid first character: %d\n", _buffer[0]);
		return ERR;
	}

	if (WebservUtility::strtoul(_buffer, _chunk_size, 16) == ERR)
	{
		printf("chunksize overflowed\n");
		return ERR;
	}


	printf("Chunk size: %lu\n", _chunk_size);

	printf("internal buffer before clearing: [%s]\n", _buffer.c_str());
	_buffer.clear();

	printf("Next: %d %c\n", buffer[_index], buffer[_index]);
	if (_chunk_size == 0)
	{
		_next_state = TRAILER;
	}
	else
	{
		_next_state = DATA;
	}
	_state = ENDLINE;
	return OK;
}

int ChunkedParser::parseChunkData(std::string const & buffer, std::string & body)
{
	printf("parseChunkData: %lu\n", _chunk_size);
	if (buffer.size() - _index >= _chunk_size)
	{
		printf("Buffer contains at least the entire chunk\n");
		body.append(buffer, _index, _chunk_size);
		_next_state = SIZE;
		_state = ENDLINE;
		_index += _chunk_size;
	}
	else
	{
		printf("Buffer contains a partial chunk\n");
		body.append(buffer, _index);
		_chunk_size -= buffer.size() - _index;
		_index = buffer.size();
	}
	return OK;
}

int ChunkedParser::parseTrailer(std::string const & buffer)
{
	if (buffer.find(CRLF, _index) == std::string::npos)
	{
		_buffer.append(buffer, _index);
		_index = buffer.size();
		return OK;
	}

	_index = buffer.find(CRLF, _index);
	_buffer.clear();
	parseEndLine(buffer);
	if (buffer.compare(_index, 2, CRLF) == 0)
	{
		parseEndLine(buffer);
		_state = FINISHED;
	}
	return OK;
}

int ChunkedParser::parseEndLine(std::string const & buffer)
{
	if (_state == ENDLINE)
	{
		_state = SIZE;
	}
	// one byte edgecase
	_state = _next_state;
	if (buffer.size() - _index == 1)
	{
		_index += 1;
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

	if (buffer.compare(_index, 2, CRLF) != 0)
	{
		return ERR;
	}
	_index += 2;
	return OK;
}

/*
Checks whether there's a CRLF in the buffer, or if the leftover has a '\r' at the back and buffer
has '\n' at the front
*/
bool ChunkedParser::hasCRLF(std::string const & buffer)
{
	if (buffer.find(CRLF, _index) == std::string::npos
	&& (_buffer.size() == 0 || !(_buffer[_buffer.size() - 1] == '\r' && buffer[0] == '\n')))
	{
		return false;
	}

	return true;
}


/*
Static (non-member) Functions
*/

ssize_t ChunkedParser::send(int fd, std::string const & body)
{
	return 0;
}


/* Debugging */

std::string ChunkedParser::getStateString() const
{
	switch (_state)
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

void ChunkedParser::print(const std::string& buffer) const
{
	printf(RED_BOLD "ChunkedParser" RESET_COLOR "\n");
	printf("State: [%s]\n", getStateString().c_str());
	printf("Index: [%lu]\n", _index);
	printf("Chunk Size: [%lu]\n", _chunk_size);
	printf("Internal Buffer: [%s]\n", _buffer.c_str());
	printf("External buffer:\n");
	printf("[%s]\n", buffer.c_str());
}
