#pragma once
#include <sys/socket.h>
#include "AFdInfo.hpp"
#include <string>
#include "parser/RequestParser.hpp"
#include "handler/Handler.hpp"

class File;

class Client : public AFdInfo
{
	public:
		Client(int fd);
		struct pollfd getPollFd() const;
		int		writeEvent(FdTable & fd_table);
		int		readEvent(FdTable & fd_table);
		int		closeEvent();

	private:
		//TODO: what TODO with client information (through accept()??)
		//TODO: add time last active for TIMEOUT
		Handler				_handler;
};
