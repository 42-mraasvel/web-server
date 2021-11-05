#pragma once

#include "fd/AFdInfo.hpp"
#include "parser/Request.hpp"

/*
Main functionality of this class:

	- FD to write to the CGI process
	- Writes the request message-body to WRITE-end of pipe
*/

class CgiSender : public AFdInfo
{
	public:
		CgiSender(int fd, Request* r);
		~CgiSender();

		struct pollfd getPollFd() const;
		int writeEvent(FdTable & fd_table);
		int readEvent(FdTable & fd_table);

	private:
		Request* _request;
};
