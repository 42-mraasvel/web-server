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

int strtol(const char* s, long& n, int base = 10);
int strtol(std::string const & s, long& n, int base = 10);
int strtoul(std::string const & s, unsigned long& n, int base = 10);
int strtoul(const char* s, unsigned long& n, int base = 10);

long strtol(const char* s, int base = 10);
long strtol(std::string const & s, int base = 10);
unsigned long strtoul(std::string const & s, int base = 10);
unsigned long strtoul(const char* s, int base = 10);

std::string itoa(unsigned long n, unsigned int base = 10);
std::string itoa(unsigned int n, unsigned int base = 10);
std::string itoa(long n, unsigned int base = 10);
std::string itoa(int n, unsigned int base = 10);

bool isBase(char x, int base);

void convertToLowercase(std::string & s);
std::string strToLower(std::string const & s);
bool caseInsensitiveEqual(const std::string& a, const std::string& b);

bool stringEndsWith(const std::string &s, const std::string& to_find,
                std::size_t begin = 0, std::size_t end = std::string::npos);

void closePipe(int* fds);
char** getEnvp();
int makeNonBlocking(int fd);
const char*	ft_basename(const char* x);
bool additionOverflow(std::size_t x, std::size_t y);

bool	isFileExisted(std::string const & file_path);
bool	isDirectoryExisted(std::string const & directory_path);
int		createDirectories(std::string const & path);
int 	list_directory(std::string const & directory, std::string const & path, std::string & content);
std::string trimPort(std::string const & str);

}

int syscallError(const std::string& str);
void abortProgram();
void abortProgram(std::string const & message);

bool isSeperator(char x);
bool isControl(char x);
bool isTokenChar(char x);
bool isPchar(char x);
bool isQueryChar(char x);
bool isDigit(char x);
bool isWhiteSpace(char x);
bool isVchar(char x);
bool isHex(char x);
