#include "http_bnf.hpp"
#include "utility/utility.hpp"
#include <string>

bool isSeperator(char x)
{
	static const std::string seperators(SEPERATORS);
	return seperators.find(x) != std::string::npos;
}

bool isControl(char x)
{
	return (x >= 0 && x < 32) || x == 127;
}

bool isTokenChar(char x)
{
	return !isControl(x) && !isSeperator(x);
}

bool isVchar(char x)
{
	return !isControl(x) && !isWhiteSpace(x);
}


bool isPchar(char x)
{
	static const std::string pchars(PCHAR);
	return pchars.find(x) != std::string::npos;
}

bool isQueryChar(char x)
{
	return x == '/' || x == '?' || isPchar(x);
}

bool isWhiteSpace(char x)
{
	static const std::string whitespace(WHITESPACE);
	return whitespace.find(x) != std::string::npos;
}

bool isDigit(char x)
{
	return x >= '0' && x <= '9';
}
