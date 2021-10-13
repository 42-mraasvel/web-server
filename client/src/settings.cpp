#include "settings.hpp"
#include <sstream>
#include <vector>
#include <algorithm>
#include <utility>
#include <map>
#include <iostream> // RM

Settings::Settings() {
	host = DEFAULT_HOST;
	port = DEFAULT_PORT;
}

int Settings::parseFlags(int argc, char *argv[])
{
	std::map<std::string, FlagParseT> dispatchers;

	dispatchers.insert(std::make_pair("-h", &Settings::parseHost));
	dispatchers.insert(std::make_pair("-p", &Settings::parsePort));


	int i;
	for (i = 1; i < argc; ++i)
	{
		std::map<std::string, FlagParseT>::iterator it = dispatchers.find(std::string(argv[i]));
		if (it != dispatchers.end()) {
			if (lastArgument(argc, i)) {
				return putError("%s: expected value\n", argv[i]);
			}
			// Dispatch
			if ((this->*(it->second))(argv[i + 1]) == ERR) {
				return ERR;
			}
			i += 1;
		} else {
			break;
		}
	}
	return i;
}

/* Flag Parsers */

int Settings::parsePort(const char *x)
{
	std::stringstream ss;
	std::string y = x;
	ss << y;
	ss >> port;
	return OK;
}

int Settings::parseHost(const char *x)
{
	host = std::string(x);
	if (host == "localhost") {
		host = "127.0.0.1";
	}
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

bool Settings::lastArgument(int argc, int i)
{
	return i == argc - 1;
}

