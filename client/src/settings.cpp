#include "settings.hpp"
#include "color.hpp"
#include "utility.hpp"
#include <cstdio>
#include <sstream>
#include <vector>
#include <algorithm>
#include <utility>
#include <iostream> // RM

Settings::Settings()
: host(DEFAULT_HOST),
port(DEFAULT_PORT),
single_connection(DEFAULT_SINGLE_CONNECTION),
read_stdin(DEFAULT_READ_STDIN),
buffer_size(DEFAULT_BUFFER_SIZE)
{}

int Settings::parseFlags(int argc, const char * const*argv)
{
	MapType dispatchers;

	dispatchers.insert(std::make_pair("-h", &Settings::parseHost));
	dispatchers.insert(std::make_pair("-p", &Settings::parsePort));
	dispatchers.insert(std::make_pair("-c", &Settings::parseConnection));
	dispatchers.insert(std::make_pair("-connection", &Settings::parseConnection));
	dispatchers.insert(std::make_pair("-stdin", &Settings::parseStdin));
	dispatchers.insert(std::make_pair("-size", &Settings::parseBufferSize));
	dispatchers.insert(std::make_pair("-buffer_size", &Settings::parseBufferSize));

	int i;
	for (i = 1; i < argc; ++i)
	{
		MapType::iterator it = dispatchers.find(std::string(argv[i]));
		if (it != dispatchers.end()) {
			// Dispatch
			if ((this->*(it->second))(argv + i, lastArgument(argc, i), i) == ERR) {
				return ERR;
			}
		} else {
			break;
		}
	}
	return i;
}

/* Flag Parsers */

int Settings::parsePort(const char * const*x, bool last_arg, int& index)
{
	if (last_arg)
	{
		return putError("Expected port: -p [PORT_NUM]\n");
	}
	std::stringstream ss;
	std::string y = *(x + 1);
	ss << y;
	ss >> port;
	index += 1;
	return OK;
}

int Settings::parseHost(const char * const*x, bool last_arg, int& index)
{
	if (last_arg)
	{
		return putError("Expected host IP: -h [HOST_ADDR]\n");
	}
	host = std::string(*(x + 1));
	if (host == "localhost") {
		host = "127.0.0.1";
	}
	index += 1;
	return OK;
}

int Settings::parseConnection(const char * const* x, bool last_arg, int& index)
{
	if (last_arg)
	{
		return putError("Expected connection option: %s [\"SINGLE\" | \"MULTIPLE\"]\n", *x);
	}

	if (caseInsensitiveEqual(*(x + 1), "SINGLE")) {
		single_connection = true;
	} else if (caseInsensitiveEqual(*(x + 1), "MULTIPLE")) {
		single_connection = false;
	} else {
		return putError("Invalid connection option: %s \n", *(x + 1));
	}
	index += 1;
	return OK;
}

int Settings::parseStdin(const char * const* x, bool last_arg, int& index)
{
	(void)last_arg;
	(void)x;
	(void)index;
	read_stdin = true;
	return OK;
}

int Settings::parseBufferSize(const char * const* x, bool last_arg, int& index)
{
	if (last_arg)
	{
		return putError("Expected buffersize: -size [BUFFER_SIZE]\n");
	}
	buffer_size = strtoul(*(x + 1), NULL, 10);
	if (errno == ERANGE)
	{
		return putError("strtoul: overflow: [%s]\n", *(x + 1));
	}
	if (buffer_size == 0 || buffer_size > MAX_BUFFER_SIZE)
	{
		return putError("Invalid BUFFER_SIZE: [%lu], Max: [%d]\n", buffer_size, MAX_BUFFER_SIZE);
	}
	index += 1;
	return OK;
}

/* Utils */

const std::string& Settings::getHost() const
{
	return host;
}

int Settings::getPort() const
{
	return port;
}

bool Settings::getSingleConnection() const
{
	return single_connection;
}

bool Settings::getStdin() const
{
	return read_stdin;
}

std::size_t Settings::getBufferSize() const
{
	return buffer_size;
}

bool Settings::lastArgument(int argc, int i)
{
	return i == argc - 1;
}

/* Debugging Parsing */

void Settings::print() const
{
	printf(CYAN_BOLD "Settings" RESET_COLOR "\n");
	printf(MAGENTA_BOLD  "  Port" RESET_COLOR " [%d]\n"
	MAGENTA_BOLD "  Host" RESET_COLOR " [%s]\n"
	MAGENTA_BOLD "  Single Connection" RESET_COLOR " [%s]\n"
	MAGENTA_BOLD "  Read STDIN" RESET_COLOR " [%s]\n"
	MAGENTA_BOLD "  BufferSize" RESET_COLOR " [%lu]\n"
	, port, host.c_str(), single_connection ? "true" : "false"
	, read_stdin ? "true" : "false", buffer_size);

	printf("\n");
}
