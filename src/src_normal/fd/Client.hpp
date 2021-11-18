#pragma once
#include <sys/socket.h>
#include <utility>
#include <string>
#include <deque>
#include "FdTable.hpp"
#include "AFdInfo.hpp"
#include "handler/RequestHandler.hpp"
#include "handler/Response.hpp"
#include "parser/Request.hpp"

class File;

class Client : public AFdInfo
{
	public:
		Client(int fd, Config::ip_host_pair address, Config::address_map* config_map);
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
		bool		isRequestExecuted() const;
		void	resetRequest();

	/* write*/
	public:
		int	writeEvent(FdTable & fd_table);
	private:
		bool	retrieveResponse();
		void	processResponse();
		void		appendResponseString();
		void	evaluateConnection();
		void		closeConnection();
		void	resetResponse();
		int		sendResponseString();

	/* utility */
	public:
		typedef RequestParser::header_field_t::iterator header_iterator;
		typedef std::deque< Response * >	ResponseQueue;
		void	updateEvents(AFdInfo::EventTypes type, FdTable & fd_table);
		void	update(FdTable & fd_table);
	
	/* Debugging */
	public:
		std::string getName() const;
	private:
		bool	isResponseReadyToWrite() const;

	private:
		Config::ip_host_pair 	_address;
		Config::address_map* 	_config_map;
		RequestHandler			_request_parser;
		Request*				_request;
		Response*				_new_response;
		ResponseQueue			_response_queue;
		Response*				_response;
		std::string				_response_string;
};
