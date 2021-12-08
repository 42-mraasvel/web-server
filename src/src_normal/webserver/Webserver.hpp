#pragma once
#include "fd/FdTable.hpp"
#include "config/Config.hpp"

class Webserver
{
	public:
		typedef Config::ip_host_pair ip_host_pair;
		typedef Config::address_map address_map;
	public:
		Webserver(Config::address_map map);
		int init();
		int	run();

	private:
		int initServer(Config::ip_host_pair ip_host_pair);
		bool shouldInitialize(ip_host_pair const & iphost) const;
		int	dispatchFd(int ready);
		void scanFdTable();
		bool shouldExecuteFd(const FdTable::AFdPointer afd);
		bool shouldCloseFd(short revents) const;
		void executeFd(short revents, FdTable::AFdPointer afd);

		void printOpening() const;

	private:
		FdTable 			_fd_table;
		address_map			_config_map;
	
	/* Debugging */
	public:
		void print() const;
};
