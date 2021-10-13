#pragma once

#include "color.hpp"
#include <string>

#define ERR -1
#define OK 0

#define PRINT 1

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 8080

int putError(const char* format, ...);

class Settings {
public:
	Settings();

	int parseFlags(int argc, char *argv[]);

	const std::string& getHost() const;
	int getPort() const;
private:
	bool lastArgument(int argc, int i);

	typedef int (Settings::*FlagParseT)(const char *);

	int parsePort(const char *x);
	int parseHost(const char *x);
private:
	std::string host;
	int port;
};
