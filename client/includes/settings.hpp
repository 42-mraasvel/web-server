#pragma once

#include "color.hpp"
#include "constants.hpp"
#include <string>
#include <map>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 8080
#define DEFAULT_SINGLE_CONNECTION true
#define DEFAULT_READ_STDIN false
#define DEFAULT_BUFFER_SIZE BUFFER_SIZE
#define DEFAULT_READ_TIMEOUT 5 // in seconds

// 10 MB
#define MAX_BUFFER_SIZE 1048576 * 10

int putError(const char* format, ...);

class Settings {
public:
	Settings();

	int parseFlags(int argc, const char * const*argv);

	const std::string& getHost() const;
	int getPort() const;
	bool getSingleConnection() const;
	bool getStdin() const;
	std::size_t getBufferSize() const;
	int getReadTimeout() const;
private:
	bool lastArgument(int argc, int i);
	bool validSettings();

	typedef int (Settings::*FlagParseType)(const char * const*, bool, int&);
	typedef std::map<std::string, FlagParseType> MapType;

	int parsePort(const char * const* x, bool last_arg, int& index);
	int parseHost(const char * const* x, bool last_arg, int& index);
	int parseConnection(const char * const* x, bool last_arg, int& index);
	int parseStdin(const char * const* x, bool last_arg, int& index);
	int parseBufferSize(const char * const* x, bool last_arg, int& index);
	int parseReadTimeout(const char * const* x, bool last_arg, int& index);

public:
	/* Debugging */
	void print() const;
private:
	std::string host;
	int port;
	bool single_connection;
	bool read_stdin;
	std::size_t buffer_size;
	int read_timeout;
};
