#pragma once
#include "AFdInfo.hpp"
#include <vector>

//TODO: Modify server FD to contain a vector of server_configurations
//TODO: add vector for ServerConfig to give port number resolution
//(first one is the default, rest is based on hostname resolution)
class Server : public AFdInfo
{
	public:
		int	setupServer(std::pair<std::string, int> address);
		int	readEvent(FdTable & fd_table);
		int	writeEvent(FdTable & fd_table);
		int closeEvent();
		struct pollfd getPollFd() const;
};
