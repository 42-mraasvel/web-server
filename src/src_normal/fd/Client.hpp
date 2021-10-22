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
		int		processRequest(FdTable & fd_table);
		void		initResponse();
		void	resetRequest();

	/* write*/
	public:
		int	writeEvent(FdTable & fd_table);
	private:
		bool	retrieveResponse();
		int		processResponse();
		void	appendResponseString();
		void	resetResponse();
		int		sendResponseString();

	/* close */
	public:
		int		closeEvent();

	/* utility */
	public:
		typedef RequestParser::header_field_t::iterator header_iterator;
		void	updateEvents(AFdInfo::EventTypes type, FdTable & fd_table);
		void	update(FdTable & fd_table);

	private:
		RequestParser			_request_parser;
		Request*				_request;
		Response*				_new_response;
		std::queue<Response *>	_response_queue;
		Response*				_response;
		std::string				_response_string;


};
