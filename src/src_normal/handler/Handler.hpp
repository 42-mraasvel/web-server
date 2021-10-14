#pragma once
#include "parser/RequestParser.hpp"
#include "fd/FdTable.hpp"
#include "fd/AFdInfo.hpp"

class Client;

class File;

class Handler
{
	/* processRequest */
    public:
		int	processRequest(FdTable & fd_table);

	private:
		void	previewMethod();

		int		parseRequest();
		void	generateAbsoluteTarget();

        int 	executeMethod(FdTable & fd_table);
		int		createFile();
		int		insertFile(FdTable & fd_table);
        int 	methodGet();
        int 	methodPost();
        int 	methodDelete();
		void	resetBuffer();

	/* sendResponse */
	public:
		int	sendResponse(FdTable & fd_table);

	private:
		void	generateHeaderString();
		void	generateResponse();

	/* utility */
	public:
		Handler();
		typedef RequestParser::header_field_t::iterator header_iterator;
		void	setClient(Client* client);

	private:
		std::string	ft_itoa(int	i) const;

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
		std::string	_status_code;
		std::string	_status_phrase;
		std::string _header_string;
		std::string	_message_body;



};

