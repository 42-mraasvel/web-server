#include "Server.hpp"
#include <iostream>
#include "settings.hpp"
#include <poll.h>
#include <netinet/in.h>
#include <cstdio>
#include <sys/socket.h>

int		Server::initServer(int port)
{
	this->_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_fd == ERR)
	{
		perror("Socket Error");
		return ERR;
	}
	sockaddr_in	address;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(this->_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == ERR)
	{
		perror("Bind Error");
		return ERR;
	}
	if (listen(this->_fd, BACKLOG) == ERR)
	{
		perror("Listen Error");
		return ERR;
	}
	return OK;
}

int Server::readEvent()
{
	return OK;
}

int Server::writeEvent()
{
	return OK;
}

struct pollfd Server::getPollFd() const
{
	struct pollfd temp;
	temp.fd = this->_fd;
	temp.events = POLLIN;
	temp.revents = 0;
	return (temp);
}







