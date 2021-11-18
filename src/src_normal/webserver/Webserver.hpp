#pragma once
#include "fd/FdTable.hpp"
#include "config/Config.hpp"

class Webserver
{
	public:
		Webserver(Config::address_map map);
		int init(Config const & config);
		int	run();

	private:
		int initServer(ConfigServer const & conf);
		int	dispatchFd(int ready);
		void	scanFdTable();
		int checkDisconnectedSockets();

	private:
		FdTable 			_fd_table;
		Config::address_map	_config_map;
	
	/* Debugging */
	public:
		void print() const;
};
