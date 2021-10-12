#pragma once
#include "AFdInfo.hpp"
#include <string>

class Client;

class File : public AFdInfo
{
	public:
		File(Client *client, int fd);
		struct pollfd	getPollFd() const;

		int	readEvent(FdTable & fd_table);
		int	writeEvent(FdTable & fd_table);
		int	closeEvent();

		std::string	const &	getContent() const;

	private:
		Client*		_client;
		std::string	_content;
};
