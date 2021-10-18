#pragma once
#include <sys/socket.h>
#include <string>
#include "fd/FdTable.hpp"
#include "fd/AFdInfo.hpp"
#include "parser/RequestParser.hpp"

struct Response
{
	public:
		RequestParser::header_field_t  header_fields;

		std::string	response;
		std::string http_version;
		int			status_code;
		std::string header_string;
		std::string	message_body;
};

class File;

class Client : public AFdInfo
{
	public:
		Client(int fd);
		struct pollfd getPollFd() const;

	/* write - process request */
	public:
		int		writeEvent(FdTable & fd_table);
	private:
		/* step 1 parse */
		int	parseRequest();
		void	generateAbsoluteTarget();

		/* step 2 check error */
		int	checkError();
		void	checkHttpVersion();
		void	checkMethod();
		void	checkContentLength();

		/* step 3 execute */
        int executeMethod(FdTable & fd_table);
		void	previewMethod();
		int		createFile();
		int		insertFile(FdTable & fd_table);
        int 	methodGet();
        int 	methodPost();
        int 	methodDelete();
        int 	methodOther();
		void	resetBuffer();

	/* read - send response*/
	public:
		int		readEvent(FdTable & fd_table);
	private:
		void	generateResponse();
		int		responseGet();
		int		responsePost();
		int		responseDelete();
		int		responseOther();

		void	setHttpVersion();
		void	setHeaderString();
		void	setResponse();

	/* close */
	public:
		int		closeEvent();

	/* utility */
	public:
		typedef RequestParser::header_field_t::iterator header_iterator;

	private:
		//TODO: what TODO with client information (through accept()??)
		//TODO: add time last active for TIMEOUT
		int					_oflag;
		AFdInfo::EventTypes	_file_event;
		std::string	_absolute_target;
		std::string	_request;

		RequestParser		_request_parser;
		File*				_file;
		Response			_response;
};
