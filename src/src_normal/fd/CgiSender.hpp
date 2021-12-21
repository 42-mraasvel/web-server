#pragma once

#include "AFdInfo.hpp"
#include "request/Request.hpp"

class Timer;

/*
Main functionality of this class:

	- FD to write to the CGI process
	- Writes the request message-body to WRITE-end of pipe
	- Set flags to remove this class when it is finished writing
*/

class CgiSender : public AFdInfo
{
	private:
		CgiSender(CgiSender const & rhs);
		CgiSender & operator=(CgiSender const & rhs);

	public:
		CgiSender(int fd, SmartPointer<Request> r, Timer* timer);
		~CgiSender();

		struct pollfd getPollFd() const;
		void writeEvent(FdTable & fd_table);
		void readEvent(FdTable & fd_table);
		void closeEvent(FdTable & fd_table);
		void exceptionEvent(FdTable & fd_table);

		int getStatusCode() const;
		void clear();

	public:
		std::string getName() const;

	private:

		void closeEvent(FdTable & fd_table, AFdInfo::Flags flag, int status_code);

	private:
		std::string _message_body;
		int _status_code;
		Timer* _timer;
};
