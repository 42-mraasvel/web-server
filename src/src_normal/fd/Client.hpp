#pragma once
#include <sys/socket.h>
#include "AFdInfo.hpp"
#include <string>

class Client : public AFdInfo
{
	public:
		enum EventTypes
		{
			WRITING,
			WAITING,
			READING
		};
	public:
		Client(int fd);
		struct pollfd getPollFd() const;
		int		writeEvent(FdTable & fd_table);
		int		readEvent(FdTable & fd_table);
		int		closeEvent();


	private:
		void	updateEvents(Client::EventTypes type, FdTable & fd_table);

	private:
		struct sockaddr *	_address;
		socklen_t			_address_len;
		std::string			_request;
		// Parser				parser;
		// Executor			executor;
		// File*				file;
};
