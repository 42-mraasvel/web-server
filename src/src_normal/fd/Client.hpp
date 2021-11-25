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
		typedef	ConfigResolver::MapType		MapType;
		typedef	ConfigResolver::AddressType	AddressType;
		typedef RequestHandler::RequestPointer RequestPointer;
		typedef SmartPointer<Response> ResponsePointer;

	public:
		Client(int fd, AddressType client, AddressType interface, MapType const * config_map);
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
		bool			isRequestComplete() const;
		bool			isRequestExecuted() const;
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
		void	removeWriteEvent(FdTable & fd_table);
	/* utility */
	public:
		typedef HeaderField::iterator header_iterator;
		typedef std::deque< ResponsePointer >	ResponseQueue;
		void	updateEvents(AFdInfo::EventTypes type, FdTable & fd_table);
		void	update(FdTable & fd_table);
	
	/* Debugging */
	public:
		std::string getName() const;
	private:
		bool	isResponseReadyToWrite() const;

	private:
		MapType const * 		_config_map;
		RequestHandler			_request_handler;
		RequestPointer			_request;
		ResponsePointer			_new_response;
		ResponseQueue			_response_queue;
		ResponsePointer			_response;
		std::string				_response_string;
		bool					_close_connection;
};
