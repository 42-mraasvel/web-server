#pragma once
#include "AFdInfo.hpp"
#include <vector>

//TODO: Modify server FD to contain a vector of server_configurations
//(first one is the default, rest is based on hostname resolution)
class Server : public AFdInfo
{
	public:
		int	setupServer(int port);
		int	readEvent(FdTable & fd_table);
		int	writeEvent(FdTable & fd_table);
		int closeEvent();
		struct pollfd getPollFd() const;
};
