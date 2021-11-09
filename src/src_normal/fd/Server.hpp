#pragma once
#include "AFdInfo.hpp"
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

//TODO: Modify server FD to contain a vector of server_configurations
//TODO: add vector for ServerConfig to give port number resolution
//(first one is the default, rest is based on hostname resolution)
class Server : public AFdInfo
{
	public:
		int	setupServer(int port);
		int	readEvent(FdTable & fd_table);
		int	writeEvent(FdTable & fd_table);
		struct pollfd getPollFd() const;
	private:
		int	initClient(sockaddr_in address, int connection_fd, FdTable & fd_table);
		int	convertIP(sockaddr_in address, std::string & ip);
	
	/* Debugging */
	public:
		std::string getName() const;
	
	private:
		int	_port;
};
