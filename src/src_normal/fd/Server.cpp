#include "Server.hpp"
#include "settings.hpp"
#include "Connection.hpp"
#include "utility/utility.hpp"
#include <poll.h>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>

Server::Server() {}
Server::~Server() {}

int		Server::setupServer(Config::ip_host_pair ip_host_pair, Config::address_map* config_map)
{
	this->_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_fd == ERR)
	{
		return syscallError(_FUNC_ERR("socket"));
	}
	sockaddr_in	address;
	address.sin_family = AF_INET;
	address.sin_port = htons(ip_host_pair.second);
	address.sin_addr.s_addr = inet_addr(ip_host_pair.first.c_str());
	if (bind(this->_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == ERR)
	{
		return syscallError(_FUNC_ERR("bind"));
	}
	if (listen(this->_fd, BACKLOG) == ERR)
	{
		return syscallError(_FUNC_ERR("listen"));
	}
	if (WebservUtility::makeNonBlocking(_fd) == ERR)
	{
		return syscallError(_FUNC_ERR("fcntl"));
	}
	_port = ip_host_pair.second; //TODO: to evaluate later
	_config_map = config_map;
	return OK;
}

void Server::readEvent(FdTable & fd_table)
{
	sockaddr_in	client_address;
	socklen_t	address_len = sizeof(client_address);
	int connection_fd = accept(_fd, reinterpret_cast<sockaddr *>(&client_address), &address_len);
	if (connection_fd == ERR)
	{
		syscallError(_FUNC_ERR("accept"));
		return;
	}
	if (fcntl(connection_fd, F_SETFL, O_NONBLOCK) == ERR)
	{
		syscallError(_FUNC_ERR("fcntl"));
		close(connection_fd);
		return;
	}
#ifdef __APPLE__
	int enable = 1;
	if (setsockopt(connection_fd, SOL_SOCKET, SO_NOSIGPIPE, &enable, sizeof(int)) < 0)
	{
		syscallError(_FUNC_ERR("setsockopt"));
		close(connection_fd);
		return;
	}
#endif

	try
	{
		initClient(client_address, connection_fd, fd_table);
	}
	catch (std::exception const & e)
	{
		close(connection_fd);
		throw;
	}
}

int	Server::initClient(sockaddr_in address, int connection_fd, FdTable & fd_table)
{
	std::string ip;
	if (convertIP(address, ip) == ERR)
	{
		return ERR;
	}

	Config::ip_host_pair	address_output;
	address_output.first = ip;
	address_output.second = _port;

	// We have to use the interface IP to match with the correct server block
	Config::ip_host_pair interface_address;
	if (getSocketAddress(connection_fd, interface_address) == ERR)
	{
		return ERR;
	}

	SmartPointer<Connection> connection(new Connection(connection_fd, address_output, interface_address, _config_map));
	fd_table.insertFd(SmartPointer<AFdInfo>(connection));
	return OK;
}

/*
Get the interface information from a socketFd (IP:PORT belonging to the interface)
This gives you the IP:PORT the client connected to, useful for INADDR_ANY (0.0.0.0) sockets
*/
int Server::getSocketAddress(int sockfd, Config::ip_host_pair & dst)
{
	sockaddr_in interface_addr;
	socklen_t len = sizeof(interface_addr);
	memset(&interface_addr, 0, len);
	if (getsockname(sockfd, reinterpret_cast<sockaddr *> (&interface_addr), &len) == ERR)
	{
		return syscallError(_FUNC_ERR("getsockname"));
	}

	dst.second = ntohs(interface_addr.sin_port);
	return convertIP(interface_addr, dst.first);
}

int	Server::convertIP(sockaddr_in address, std::string & ip)
{
	std::string full_ip(INET_ADDRSTRLEN, '\0');
	if (!inet_ntop(AF_INET, &(address.sin_addr), &full_ip[0], INET_ADDRSTRLEN))
	{
		syscallError(_FUNC_ERR("inet_ntp"));
		return ERR;
	}
	size_t found = full_ip.find_first_of('\0');
	ip = full_ip.substr(0, found);
	return OK;
}

void Server::writeEvent(FdTable & fd_table)
{
	(void)fd_table;
	abortProgram(RED_BOLD "SERVER WRITE EVENT: ABORTING" RESET_COLOR);
}

void Server::update(FdTable & fd_table)
{
	if (fd_table.size() < FD_TABLE_MAX_SIZE)
	{
		updateEvents(AFdInfo::READING, fd_table);
	}
	else
	{
		updateEvents(AFdInfo::WAITING, fd_table);
	}
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
