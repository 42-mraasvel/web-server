#pragma once
#include "AFdInfo.hpp"
#include <vector>

class Server : public AFdInfo
{
	public:
		int	setupServer(int port);
		int	readEvent(FdTable & fd_table);
		int	writeEvent(FdTable & fd_table);
		int closeEvent();
		struct pollfd getPollFd() const;
};
