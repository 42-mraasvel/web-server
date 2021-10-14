#pragma once
#include "parser/RequestParser.hpp"
#include "fd/FdTable.hpp"
#include "fd/AFdInfo.hpp"

class Client;

class File;

class Handler
{
	public:
		Handler();

	/* processRequest */
    public:
		int	processRequest(FdTable & fd_table);

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

	/* sendResponse */
	public:
		int	sendResponse(FdTable & fd_table);

	private:
		void	generateResponse();
		int		responseGet();
		int		responsePost();
		int		responseDelete();
		int		responseOther();

		void	setHttpVersion();
		void	setHeaderString();
		void	setResponse();

	/* utility */
	public:
		typedef RequestParser::header_field_t::iterator header_iterator;
		void	setClient(Client* client);

	/* attribute */
    private:
		int					_oflag;
		AFdInfo::EventTypes	_file_event;
		RequestParser		_request_parser;
		File*				_file;
		Client*				_client;

		RequestParser::header_field_t  _header_fields;
		std::string	_absolute_target;

		std::string	_request;
		std::string	_response;
		std::string _http_version;
		int			_status_code;
		std::string _header_string;
		std::string	_message_body;

};

