#pragma once

#include <string>

struct Address {
	Address(const std::string& ip, int port);

	std::string ip;
	int port;
};
