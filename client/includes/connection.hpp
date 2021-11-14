#pragma once

#include <string>

class Connection {
public:
	int initConnection(const std::string& server_ip, int port, bool blocking);

	ssize_t sendRequest(const std::string& msg, std::size_t size);
	ssize_t receiveResponse(std::string& storage) const;
	void closeConnection();

private:
	int connection_fd;
};
