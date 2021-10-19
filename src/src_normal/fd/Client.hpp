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
		int		readEvent(FdTable & fd_table);
	private:
		/* step 1 parse */
		int	parseRequest();
		int		readRequest(std::string & buffer);

		/* step 2 retrieve request */
		bool	retrieveRequest();

		/* step 3 process request */
		int	processRequest(FdTable & fd_table);

		/* step 3.1 init response */
		void	initResponse();

		/* step 3.2 check request error */
		bool	isRequestError();
		bool		checkBadRequest();
		bool		checkHttpVersion();
		bool		checkMethod();
		bool		checkContentLength();

		/* step 3.3 setup file */
		int		setupFile(FdTable & fd_table);

		/* step 3.4 execute */
        int 	executeMethod(FdTable & fd_table);
        int 		methodGet(FdTable & fd_table);
        int 		methodPost(FdTable & fd_table);
        int 		methodDelete(FdTable & fd_table);
        int 		methodOther(FdTable & fd_table);

		/* step 4 reset */
		void	resetRequest();

	/* write*/
	public:
		int		writeEvent(FdTable & fd_table);

		/* step 3.3 generate response */
	private:
		int		generateResponse();
		int			responseGet();
		int			responsePost();
		int			responseDelete();
		int			responseOther();

		void		setHttpVersion();
		void		setHeaderString();
		void		setResponseString();


	/* close */
	public:
		int		closeEvent();

	/* utility */
	public:
		typedef RequestParser::header_field_t::iterator header_iterator;
		void	updateEvents(AFdInfo::EventTypes type, FdTable & fd_table);
		bool	updateEventsSpecial();
		void	appendFileContent();

	private:
		Request*				_request;
		Response*				_response;
		RequestParser			_request_parser;
		std::queue<Response *>	_response_queue;


};
