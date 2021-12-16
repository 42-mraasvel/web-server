#include "HeaderFieldParser.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "utility/status_codes.hpp"
#include "ParserUtils.hpp"

HeaderFieldParser::HeaderFieldParser(ValidFieldFunction valid_field_function,
									std::size_t max_header_field_size)
:
_state(HeaderFieldParser::PARSING),
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
	while (_index < buffer.size() && _state != HeaderFieldParser::COMPLETE)
	{
		std::size_t start = _index;
		_index = WebservUtility::findEndLine(_leftover, buffer, _index);
		if (_index == std::string::npos)
		{
			_index = buffer.size();
			if (appendLeftover(buffer, start, _index) == ERR)
			{
				return ERR;
			}
		}
		else
		{
			if (parseHeaderField(buffer, start, _index) == ERR)
			{
				return ERR;
			}
			WebservUtility::skipEndLine(buffer, _index);
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
	_index = WebservUtility::findEndLine(_leftover, buffer, _index);

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
	WebservUtility::skipEndLine(buffer, _index);
	// Parse leftover's field into map
	if (parseHeaderField(_leftover, 0, _leftover.size()) == ERR)
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

	if (end - start + _leftover.size() > _max_size)
	{
		return setError(StatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE);
	}

	_leftover.append(buffer, start, end - start);
	return OK;
}

/*
Precondition: the entire header-field is present in the string s.
Index meaning the start of the header-field, and end the start of the 'CRLF' or other Newline
representation signalling the end of the header-field.
If the size of the string (end - start) equals 0: that is the end of the header-field section
*/
int HeaderFieldParser::parseHeaderField(std::string const & s, std::size_t start, std::size_t end)
{
	if (end - start > _max_size || _header.size() > MAX_HEADER_FIELDS)
	{
		return setError(StatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE);
	}
	else if (end - start == 0)
	{
		// Empty field-name means there is an EOHEADER (CRLF) at this point
		return setState(HeaderFieldParser::COMPLETE);
	}

	std::string key, value;

	if (parseFieldName(s, key, start) == ERR)
	{
		return setError(StatusCode::BAD_REQUEST);
	}
	if (skipColon(s, start) == ERR)
	{
		return setError(StatusCode::BAD_REQUEST);
	}
	// Whitespace is optional so we don't have to error-check it
	skip(s, start, isWhiteSpace);
	if (parseFieldValue(s, value, start, end) == ERR)
	{
		return setError(StatusCode::BAD_REQUEST);
	}
	// Custom field validator
	if (!_valid_field(key, value, _header))
	{
		_failed_pair = std::make_pair(key, value);
		return setError(StatusCode::BAD_REQUEST);
	}
	_header[key] = value;
	return OK;
}

int HeaderFieldParser::parseFieldName(const std::string& s,
									std::string& key, std::size_t& index) const
{
	if (!isTokenChar(s[index]))
	{
		return ERR;
	}
	std::size_t start = index;
	skip(s, index, isTokenChar);
	key = s.substr(start, index - start);
	return OK;
}

int HeaderFieldParser::skipColon(const std::string& s, std::size_t& index) const
{
	if (index >= s.size() || s[index] != ':')
	{
		return ERR;
	}

	++index;
	return OK;
}

int HeaderFieldParser::skip(const std::string& s, std::size_t& index, IsFunctionType f) const
{
	while (index < s.size() && f(s[index]))
	{
		++index;
	}
	return OK;
}

int HeaderFieldParser::parseFieldValue(const std::string& s, std::string& value,
									std::size_t& index, std::size_t end) const
{
	// Meaning there was only whitespace between colon and CRLF
	if (index == end)
	{
		return OK;
	}
	std::size_t end_value = end - 1;
	while (isWhiteSpace(s[end_value]))
	{
		--end_value;
	}
	value = s.substr(index, end_value - index + 1);
	index = end;
	for (std::size_t i = 0; i < value.size(); ++i)
	{
		if (!isWhiteSpace(value[i]) && !isVchar(value[i]))
		{
			return ERR;
		}
	}
	return OK;
}

/*
Public interfaces
*/

const std::pair<std::string, std::string>& HeaderFieldParser::getFailedPair() const
{
	return _failed_pair;
}

bool HeaderFieldParser::isError() const
{
	return _state == HeaderFieldParser::ERROR;
}

bool HeaderFieldParser::isComplete() const
{
	return _state == HeaderFieldParser::COMPLETE;
}

int HeaderFieldParser::getStatusCode() const
{
	return _status_code;
}

void HeaderFieldParser::reset()
{
	_state = HeaderFieldParser::PARSING;
	_header.clear();
	_leftover.clear();
}

HeaderFieldParser::HeaderFieldType& HeaderFieldParser::getHeaderField()
{
	return _header;
}

/*
Private utilities
*/

int HeaderFieldParser::setError(int code)
{
	_status_code = code;
	setState(ERROR);
	return ERR;
}

int HeaderFieldParser::setState(State type)
{
	_state = type;
	return OK;
}
