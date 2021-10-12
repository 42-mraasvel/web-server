#pragma once

#include <vector>
#include <string>

namespace WebservUtility
{

std::vector<std::string> splitString(	const std::string& input,
										char delim);
std::vector<std::string> splitString(	const std::string& input,
										const std::string& delim_set);

std::size_t findLimit(std::string const & s,
					std::string const & to_find,
					std::size_t limit);

long strtol(const char* s);
long strtol(std::string const & s);
unsigned long strtoul(std::string const & s);
unsigned long strtoul(const char* s);

void convertToLowercase(std::string & s);

}

bool isSeperator(char x);
bool isControl(char x);
bool isTokenChar(char x);
bool isPchar(char x);
bool isQueryChar(char x);
bool isDigit(char x);
bool isWhiteSpace(char x);
