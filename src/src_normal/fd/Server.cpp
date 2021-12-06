#include "Server.hpp"
#include "settings.hpp"
#include "Client.hpp"
#include <poll.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>

int		Server::setupServer(std::pair<std::string, int> ip_host_pair)
{
	this->_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_fd == ERR)
	{
		perror("Socket Error");
		return ERR;
	}
	sockaddr_in	address;
	address.sin_family = AF_INET;
	address.sin_port = htons(ip_host_pair.second);
	address.sin_addr.s_addr = inet_addr(ip_host_pair.first.c_str());
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
	if (fcntl(_fd, F_SETFL, O_NONBLOCK) == ERR)
	{
		perror("fcntl");
	}
	return OK;
}

int Server::readEvent(FdTable & fd_table)
{
	//TODO: in accept() store client info and map the connection
	int connection_fd = accept(_fd, NULL, NULL);
	if (connection_fd == ERR)
	{
		perror("Accept");
		return ERR;
	}
	if (fcntl(connection_fd, F_SETFL, O_NONBLOCK) == ERR)
	{
		perror("fcntl");
	}
	Client*	client = new Client(connection_fd);
	fd_table.insertFd(client);
	return OK;
}

int Server::writeEvent(FdTable & fd_table)
{
	std::cerr << RED_BOLD "Server write event detected!!" RESET_COLOR << std::endl;
	return ERR;
}

struct pollfd Server::getPollFd() const
{
	struct pollfd temp;
	temp.fd = this->_fd;
	temp.events = POLLIN;
	temp.revents = 0;
	return (temp);
}

std::string Server::getName() const
{
	return "Server";
}
