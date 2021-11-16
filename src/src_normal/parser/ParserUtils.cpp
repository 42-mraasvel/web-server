#include "ParserUtils.hpp"
#include "settings.hpp"
#include <string>

namespace WebservUtility
{

/*
Return index inside the buffer, to the start of the next endline
TODO: newline should be a custom string, i.e. it can be any representation of a string (INPUT)
So we can match a string at the end of leftover to the end of the buffer
*/
std::size_t findEndLine(std::string & leftover, std::string const & buffer, std::size_t index)
{
	// Edgecase: back of leftover has '\r', front of buffer has '\n'
	if (leftover.size() > 0 && leftover[leftover.size() - 1] == '\r' && buffer[0] == '\n')
	{
		leftover.resize(leftover.size() - 1);
		return 0;
	}

	return buffer.find(CRLF, index);
}

void skipEndLine(std::string const & buffer, std::size_t & index)
{
	if (buffer[index] == '\n')
	{
		index += 1;
	}
	else if (buffer[index] == '\r')
	{
		index += 2;
	}
}

}
