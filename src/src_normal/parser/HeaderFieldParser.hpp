#pragma once

#include "utility/HeaderField.hpp"

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
		typedef HeaderField HeaderFieldType;

		enum ErrorType
		{
			HEADER_FIELD_SIZE,
			INVALID_FIELD
		};

	private:
		enum State
		{
			PARSING,
			COMPLETE,
			ERROR
		};

	public:
		HeaderFieldParser(ValidFieldFunction valid_field_function, std::size_t max_header_field_size);

		int parse(buffer_type const & buffer, std::size_t & index);

		bool isError() const;
		bool isComplete() const;
		int getStatusCode() const;
		// Should be used when the parsing is finished, use header.swap(x) for constant copy
		HeaderFieldType& getHeaderField();
		void reset();
		const std::pair<std::string, std::string>& getFailedPair() const;

	private:

		int handleLeftover(buffer_type const & buffer);

		int appendLeftover(buffer_type const & buffer, std::size_t start, std::size_t end);
		int parseHeaderField(std::string const & s, std::size_t start, std::size_t end);

		int setError(int code);
		int setState(State type);

	private:
		typedef bool (*IsFunctionType)(char);
		int skip(const std::string& s, std::size_t& index, IsFunctionType f) const;
		int skipColon(const std::string& s, std::size_t& index) const;
		int parseFieldName(const std::string& s, std::string& key, std::size_t& index) const;
		int parseFieldValue(const std::string& s, std::string& value, std::size_t& index, std::size_t end) const;

	private:
		static const std::size_t MAX_HEADER_FIELDS = 2500;

	private:
		HeaderFieldType _header;
		State _state;

		ValidFieldFunction _valid_field;
		std::size_t _max_size;
		std::string _leftover;
		std::size_t _index;
		int _status_code;
		std::pair<std::string, std::string> _failed_pair;
};
