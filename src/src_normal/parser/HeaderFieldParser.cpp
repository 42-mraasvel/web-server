#include "HeaderFieldParser.hpp"
#include "settings.hpp"

HeaderFieldParser::HeaderFieldParser(ValidFieldFunction valid_field_function,
									std::size_t max_header_field_size)
: _state(HeaderFieldParser::PARSING),
_valid_field(valid_field_function),
_max_size(max_header_field_size) {
	_leftover.reserve(max_header_field_size);
}

/*
- If there is anything in the leftover, check if there is a newline present in the combined buffer
- If there is no newline present, add buffer to leftover: AS LONG AS it fits

- If a newline is present: send it to the header-field parser
*/
int HeaderFieldParser::parse(buffer_type const & buffer, std::size_t & index)
{
	_index = index;

	if (handleLeftover(buffer) == ERR)
	{
		return ERR;
	}

	// Parse remaining headerFields
	while (_index < buffer.size())
	{
		std::size_t start = _index;
		_index = findEndLine(buffer);
		if (_index == std::string::npos)
		{
			if (appendLeftover(buffer, start, _index - start) == ERR)
			{
				return ERR;
			}
		}
		else
		{
			if (parseHeaderField(buffer, start) == ERR)
			{
				return ERR;
			}
			skipEndLine(buffer);
		}
	}

	index = _index;

	return OK;
}

/*
Return: ERR if no header-field is present within _max_size bytes

Goal:

	- Check if there is a newline in the combined string (leftover, buffer)
	- Set index to beyond the ENDLINE in buffer
	- Check if the newline is within _max_size bytes
	- Append substr to _leftover, and parse it's header-field
	- Clear the leftover, return to the main parser
*/

int HeaderFieldParser::handleLeftover(buffer_type const & buffer)
{
	if (_leftover.size() == 0)
	{
		return OK;
	}

	std::size_t start = _index;
	// Returns the index of the start of the ENDLINE in buffer
	// Will destroy "\r" from END OF leftover IF edgecase is encountered
	_index = findEndLine(buffer);

	// Append buffer[start:end] to leftover
	// Returns ERR if it exceeds MAX_SIZE
	if (appendLeftover(buffer, start, _index) == ERR)
	{
		return ERR;
	}

	// No endline found
	if (_index == std::string::npos)
	{
		return OK;
	}

	// Set index beyond the ENDLINE in buffer
	skipEndLine(buffer);
	// Parse leftover's field into map
	if (parseHeaderField(_leftover, 0) == ERR)
	{
		return ERR;
	}
	_leftover.clear();
	return OK;
}

/*
Size check the leftover (header-field-max-size)
*/
int HeaderFieldParser::appendLeftover(buffer_type const & buffer, std::size_t start, std::size_t end)
{
	if (end == std::string::npos)
	{
		end = buffer.size();
	}

	if (end - start + _leftover.size() > _max_size)
	{
		_error_type = HeaderFieldParser::HEADER_FIELD_SIZE;
		return ERR;
	}

	_leftover.append(buffer, start, end - start);
	return OK;
}

/*
Return index inside the buffer, to the start of the next endline
TODO: newline should be a custom string, i.e. it can be any representation of a string (INPUT)
So we can match a string at the end of leftover to the end of the buffer
*/
std::size_t HeaderFieldParser::findEndLine(buffer_type const & buffer)
{
	// Edgecase: back of leftover has '\r', front of buffer has '\n'
	if (_leftover.size() > 0 && _leftover[_leftover.size() - 1] == '\r' && buffer[0] == '\n')
	{
		_leftover.resize(_leftover.size() - 1);
		return 0;
	}

	return buffer.find(CRLF, _index);
}

void HeaderFieldParser::skipEndLine(buffer_type const & buffer)
{
	if (buffer[_index] == '\n')
	{
		_index += 1;
	}
	else if (buffer[_index] == '\r')
	{
		_index += 2;
	}
}

int HeaderFieldParser::parseHeaderField(std::string const & s, std::size_t index)
{
	return OK;
}
