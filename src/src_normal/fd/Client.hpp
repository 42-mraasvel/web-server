#pragma once
#include <sys/socket.h>
#include <string>
#include <queue>
#include "FdTable.hpp"
#include "AFdInfo.hpp"
#include "Response.hpp"
#include "parser/Request.hpp"

class File;

class Client : public AFdInfo
{
	public:
		Client(int fd);
		~Client();
		struct pollfd getPollFd() const;

	/* read */
	public:
		int	readEvent(FdTable & fd_table);
	private:
		int		parseRequest();
		int			readRequest(std::string & buffer);
		bool	retrieveRequest();
		void	processRequest(FdTable & fd_table);
		void		initResponse(Request const & request);
		void		checkRequestStatus();
		bool		isRequestReadyToExecute() const;
		void	reset();
		void		resetRequest();

	/* write*/
	public:
		int	writeEvent(FdTable & fd_table);
	private:
		bool	retrieveResponse();
		void	processResponse();
		void		appendResponseString();
		void	checkConnection();
		void	resetResponse();
		void		cleanQueue();
		int		sendResponseString();

	/* utility */
	public:
		typedef RequestParser::header_field_t::iterator header_iterator;
		void	updateEvents(AFdInfo::EventTypes type, FdTable & fd_table);
		void	update(FdTable & fd_table);
	private:
		void	closeConnection();

	private:
		RequestParser			_request_parser;
		Request*				_request;
		Response*				_new_response;
		std::queue<Response *>	_response_queue;
		Response*				_response;
		std::string				_response_string;
};
