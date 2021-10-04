#pragma once
#include "config/ConfigServer.hpp"

class AFdInfo
{
	public:
		virtual	~AFdInfo();
		virtual	int	writeEvent() = 0;
		virtual	int	readEvent() = 0;
		virtual struct pollfd getPollFd() const = 0;

		AFdInfo();
		AFdInfo(int fd);
		int	getFd() const;

	protected:
		int		_fd;

};
