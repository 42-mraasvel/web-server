#include "connection.hpp"
#include "settings.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>

#ifdef __linux__
#include <cstdio>
#endif /* __linux__ */

int Connection::initConnection(const std::string& server_ip, int port, bool blocking) {
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
	if (!blocking) {
		if (fcntl(connection_fd, F_SETFL, O_NONBLOCK) == -1) {
			perror("fcntl");
			closeConnection();
			return ERR;
		}
	}
	return OK;
}

ssize_t Connection::sendRequest(const std::string& msg, std::size_t size) {
	ssize_t n = send(connection_fd, msg.c_str(), size, 0);
	if (n == -1) {
		perror("send");
	}
	return n;
}

ssize_t Connection::receiveResponse(std::string& storage) const {
	storage.resize(BUFFER_SIZE, '\0');
	ssize_t n = recv(connection_fd, &storage[0], BUFFER_SIZE, 0);
	if (n == ERR && errno == EAGAIN) {
		storage.clear();
		return 0;
	} else if (n == ERR) {
		perror("recv");
		storage.clear();
		return ERR;
	}
	storage.resize(n);
	return n;
}

void Connection::closeConnection() {
	if (close(connection_fd) == ERR) {
		perror("close");
	}
}
