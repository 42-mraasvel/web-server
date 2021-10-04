#pragma once
#include "AFdInfo.hpp"
#include <vector>

class FdTable
{
	public:
		typedef std::vector<struct pollfd> PollFdTable;
		typedef std::vector<AFdInfo *> FdVector_t;

	public:
		~FdTable();
		int insertFd(AFdInfo * info);
		int eraseFd(int fd);
		int eraseFd(PollFdTable::iterator it);

	private:
		FdTable::PollFdTable::iterator findFd(int fd);

	private:
		std::vector<struct pollfd>	_pollfd_table;
		std::vector<AFdInfo *>		_fd_info_table;
	
	/* Debuging */
	public:
		void print() const;
};
