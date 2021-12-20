#pragma once
#include "AFdInfo.hpp"
#include "config/Config.hpp"
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

class Server : public AFdInfo
{
	private:
		Server(Server const & rhs);
		Server & operator=(Server const & rhs);

	public:

		Server();
		~Server();
		int	setupServer(Config::ip_host_pair ip_host_pair, Config::address_map* config_map);
		void readEvent(FdTable & fd_table);
		void writeEvent(FdTable & fd_table);
		void update(FdTable & fd_table);
		struct pollfd getPollFd() const;
	private:
		int	initClient(sockaddr_in address, int connection_fd, FdTable & fd_table);
		int	convertIP(sockaddr_in address, std::string & ip);
		int getSocketAddress(int sockfd, Config::ip_host_pair & dst);
	
	/* Debugging */
	public:
		std::string getName() const;
	
	private:
		int	_port;
		Config::address_map*	_config_map;
};
