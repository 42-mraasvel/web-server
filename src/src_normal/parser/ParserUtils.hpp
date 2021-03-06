#pragma once

# include <string>

#define CRLF "\r\n"
#define EOHEADER CRLF CRLF

namespace WebservUtility
{
	std::size_t findEndLine(std::string & leftover, std::string const & buffer, std::size_t index);
	void skipEndLine(std::string const & buffer, std::size_t & index);

	typedef bool (*IsFunctionType)(char c);
	void skip(std::string const & buffer, std::size_t & index, IsFunctionType pred);
}
