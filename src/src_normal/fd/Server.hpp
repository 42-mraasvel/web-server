#pragma once
#include "AFdInfo.hpp"
#include <vector>

class Server : public AFdInfo
{
	public:
		int	initServer(int port);
		int	readEvent();
		int	writeEvent();
		int closeEvent();
		struct pollfd getPollFd() const;
};
