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
		int initServer(Config::ip_host_pair ip_host_pair);
		int	dispatchFd(int ready);
		void scanFdTable();
		bool shouldExecuteFd(const FdTable::AFdPointer afd);
		bool shouldCloseFd(short revents) const;
		void executeFd(short revents, FdTable::AFdPointer afd);

	private:
		FdTable 			_fd_table;
		Config::address_map	_config_map;
	
	/* Debugging */
	public:
		void print() const;
};
