#pragma once

#include "HeaderField.hpp"

/*
Configuration:
	- Custom function that is called to error check [key] [value] [header]

		- bool ValidFieldFunction(string key, string value, HeaderField header);
		- If it returns false, the HeaderFieldParser will return an error
	
	- Customize what newline representations are accepted as end of header-field:
	
		- CR LF: "\r\n"
		- LF: "\n"
		- Combination of the above

Input:
	- Function to determine validity of [key] [value]
	- Maximum number of bytes the header-field can be
	- Raw buffer at HeaderFieldParsing state
	- Index representing the current buffer position, will be modified based on what's extracted
Output: HeaderField class

Usage:

	parser.parse(buffer);
	return: OK or ERR
	ERR:
		- ValidFieldFunction returned false on a [key] [value] pair
		- Size of header-field exceeded maximum storage
	Exception on allocation failure (header-field saving)

*/

class HeaderFieldParser
{
	public:
		typedef std::string buffer_type;
		typedef bool (*ValidFieldFunction)(
			std::string const & key, std::string const & value, HeaderField const & header);
		typedef HeaderField header_field_type;
	
	enum State
	{
		PARSING,
		COMPLETE,
		ERROR
	};

	enum ErrorType
	{
		HEADER_FIELD_SIZE,
		INVALID_FIELD
	};

	public:
		HeaderFieldParser(ValidFieldFunction valid_field_function, std::size_t max_header_field_size);

		int parse(buffer_type const & buffer, std::size_t & index);

	private:

		int handleLeftover(buffer_type const & buffer);

		int appendLeftover(buffer_type const & buffer, std::size_t start, std::size_t end);
		std::size_t findEndLine(buffer_type const & buffer);
		void skipEndLine(buffer_type const & buffer);
		int parseHeaderField(std::string const & s, std::size_t start, std::size_t end);

		int setError(ErrorType type);
		int setState(State type);

	private:
		header_field_type _header;
		ErrorType _error_type;
		State _state;

		ValidFieldFunction _valid_field;
		std::size_t _max_size;
		std::string _leftover;
		std::size_t _index;
};
