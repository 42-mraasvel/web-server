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
	// while (_index < buffer.size())
	// {
	// }

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

int HeaderFieldParser::appendLeftover(buffer_type const & buffer, std::size_t start, std::size_t end)
{
	return OK;
}

std::size_t HeaderFieldParser::findEndLine(buffer_type const & buffer)
{
	return std::string::npos;
}

void HeaderFieldParser::skipEndLine(buffer_type const & buffer)
{

}

int HeaderFieldParser::parseHeaderField(std::string const & s, std::size_t index)
{
	return OK;
}
