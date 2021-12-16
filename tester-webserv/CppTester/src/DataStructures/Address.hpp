#pragma once

#include <string>

struct Address {
	Address(const std::string& ip = "localhost", int port = 8080);

	std::string ip;
	int port;
};
