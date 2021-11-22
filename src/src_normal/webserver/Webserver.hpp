#pragma once
#include "fd/FdTable.hpp"
#include "config/Config.hpp"

class Webserver
{
	public:
		int init(Config const & config);
		int	run();

	private:
		// int initServer(ConfigServer const & conf);
		int initServer(std::pair<std::string, int> ip_host_pair);
		int	dispatchFd(int ready);
		void	scanFdTable();
		int checkDisconnectedSockets();

	private:
		FdTable _fd_table;
	
	/* Debugging */
	public:
		void print() const;
};
