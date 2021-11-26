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
		// int initServer(ConfigServer const & conf);
		int initServer(std::pair<std::string, int> ip_host_pair);
		int	dispatchFd(int ready);
		void scanFdTable();
		bool shouldExecuteFd(const FdTable::AFdPointer afd);
		bool shouldCloseFd(const struct pollfd & pfd);
		void executeFd(const struct pollfd& pfd, FdTable::AFdPointer afd);

	private:
		FdTable 			_fd_table;
		Config::address_map	_config_map;
	
	/* Debugging */
	public:
		void print() const;
};
