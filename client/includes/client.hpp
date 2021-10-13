#pragma once

#include <vector>
#include "connection.hpp"
#define CRLF "\r\n"
#define EOHEADER CRLF CRLF

class Client {
public:
	typedef std::vector<Connection> container_t;
public:
	Client();
	~Client();

	int newConnection(const std::string& server_ip, int port);

	void sendRequest(int index, const std::string& request);
	std::string receiveResponse(int index);

private:
	container_t connections;

};
