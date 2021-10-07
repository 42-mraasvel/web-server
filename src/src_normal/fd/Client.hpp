#pragma once
#include <sys/socket.h>
#include "AFdInfo.hpp"
#include <string>
#include "parser/RequestParser.hpp"
#include "executor/Executor.hpp"

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
		void	resetBuffer();

	private:
		//TODO: what TODO with client information??
		struct sockaddr *	_address;
		socklen_t			_address_len;
		//TODO: add time last active for TIMEOUT
		std::string			_request;
		RequestParser		_request_parser;
		Executor			_executor;
};
