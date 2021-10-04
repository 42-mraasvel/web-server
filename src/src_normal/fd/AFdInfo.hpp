#pragma once
#include "config/ConfigServer.hpp"

class AFdInfo
{
	public:
		virtual	~AFdInfo();
		virtual struct pollfd getPollFd() const = 0;
		virtual	int	writeEvent() = 0;
		virtual	int	readEvent() = 0;
		virtual int closeEvent() = 0;

		AFdInfo();
		AFdInfo(int fd);
		int	getFd() const;

	protected:
		int		_fd;
};
