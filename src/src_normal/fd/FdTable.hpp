#pragma once
#include <vector>
#include "AFdInfo.hpp"


class FdTable
{
	public:
		int insertFd(AFdInfo * info);
		int eraseFd(int fd);
		
	private:
		std::vector<struct pollfd>	_pollfd_table;
		std::vector<AFdInfo *>		_fd_table;

	protected:
};
