#pragma once

#include "fd/AFdInfo.hpp"
#include "parser/HeaderField.hpp"
#include "CgiResponseParser.hpp"

class Timer;

/*
Purpose of the class:
	- Read the output from the CGI program
	- Parse the CGI response into message_body and header
	- Set flags when the CGI request is finished
*/
class CgiReader : public AFdInfo
{
	public:
		CgiReader(int fd, Timer* timer);
		~CgiReader();

		struct pollfd getPollFd() const;
		int	writeEvent(FdTable & fd_table);
		int	readEvent(FdTable & fd_table);
		void closeEvent(FdTable & fd_table);
		void exceptionEvent(FdTable & fd_table);
		void clear();

		int getStatusCode() const;
		std::string & getBody();
		HeaderField & getHeader();
	
	public:
		std::string getName() const;

	private:
		void closeEvent(FdTable & fd_table, AFdInfo::Flags flag);
		void closeEvent(FdTable & fd_table, AFdInfo::Flags flag, int status_code);
		void parseBuffer(FdTable & fd_table, std::string const & buffer);

	private:
		CgiResponseParser _parser;
		int _status_code;
		Timer* _timer;
};
