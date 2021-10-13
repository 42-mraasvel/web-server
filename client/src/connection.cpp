#include "connection.hpp"
#include "settings.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>

#ifdef __linux__
#include <cstdio>
#endif /* __linux__ */

int Connection::initConnection(const std::string& server_ip, int port) {
	connection_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (connection_fd == ERR) {
		perror("socket");
		return ERR;
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	
	// server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());
	// if (server_addr.sin_addr.s_addr <= 0) {
	// 	perror("inet_addr");
	// 	closeConnection();
	// 	return ERR;
	// }

	int ret = inet_pton(AF_INET, server_ip.c_str(), &(server_addr.sin_addr));
	if (ret <= 0) {
		if (ret < 0) {
			perror("inet_pton");
		} else {
			putError("inet_pton: invalid IP while parsing: %s\n", server_ip.c_str());
		}
		closeConnection();
		return ERR;
	}


	if (connect(connection_fd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr)) == ERR) {
		perror("connect");
		closeConnection();
		return ERR;
	}
	return OK;
}

void Connection::sendRequest(const std::string& msg) {
	if (send(connection_fd, msg.c_str(), msg.size(), 0) == ERR) {
		perror("send");
	}
}

std::string Connection::receiveResponse() const {
	std::string buffer(BUFFER_SIZE, '\0');
	ssize_t n = recv(connection_fd, &buffer[0], buffer.size(), 0);
	if (n == ERR) {
		perror("recv");
		return "";
	}
	return buffer.substr(0, n);
}

void Connection::closeConnection() {
	if (close(connection_fd) == ERR) {
		perror("close");
	}
}
