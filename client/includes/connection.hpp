#pragma once

#include <string>

#define BUFFER_SIZE 4096

class Connection {
public:
	int initConnection(const std::string& server_ip, int port);

	void sendRequest(const std::string& msg);
	std::string receiveResponse() const;
	void closeConnection();

private:
	int connection_fd;
};
