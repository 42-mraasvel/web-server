#pragma once
#include <sys/socket.h>
#include <utility>
#include <string>
#include <deque>
#include "FdTable.hpp"
#include "AFdInfo.hpp"
#include "handler/RequestHandler.hpp"
#include "handler/ResponseHandler.hpp"
#include "parser/Request.hpp"
#include "utility/Timer.hpp"
#include "webserver/MethodType.hpp"

class File;

class Client : public AFdInfo
{
	private:
		Client(Client const & rhs);
		Client & operator=(Client const & rhs);

	public:
		typedef	ConfigResolver::MapType		MapType;
		typedef	ConfigResolver::AddressType	AddressType;
		typedef RequestHandler::RequestPointer RequestPointer;
		typedef ResponseHandler::ResponsePointer ResponsePointer;
		

	public:
		Client(int fd, AddressType client, AddressType interface, MapType const * config_map);
		~Client();
		struct pollfd getPollFd() const;

	/* read */
	public:
		void	readEvent(FdTable & fd_table);
	private:
		int			parseRequest();
		int				readRequest(std::string & buffer);

	/* update */
	public:
		void	update(FdTable & fd_table);
	private:
		void		executeRequests(FdTable & fd_table);
		bool			canExecuteRequest(int fd_table_size) const;
		bool			retrieveRequest();
		void			resetRequest();
		void		generateResponse();
		bool			retrieveResponse();
		void			resetResponse();
		void		resetEvents(FdTable & fd_table);
		void		checkTimeOut();


	/* write*/
	public:
		void	writeEvent(FdTable & fd_table);
	private:
		int			sendResponseString();
		void		removeWriteEvent(FdTable & fd_table);
		void		evaluateConnection(FdTable & fd_table);

	/* Exception */
	public:
		void	exceptionEvent(FdTable & fd_table);

	/* utility */
	public:
		typedef HeaderField::iterator header_iterator;
	private:
		void	closeConnection();
		bool	isMethodSafe(Method::Type const & method) const;
		void	increUnsafe(Method::Type const & method);
		void	decreUnsafe(Method::Type const & method);

	/* Debugging */
	public:
		std::string getName() const;

	private:
		MapType const * 		_config_map;
		RequestHandler			_request_handler;
		RequestPointer			_request;
		ResponseHandler			_response_handler;
		ResponsePointer			_response;
		std::string				_response_string;
		bool					_close_connection;
		bool					_close_timer_set;
		Timer					_timer;
		int						_unsafe_request_count;
};
