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

		typedef RequestParser::header_field_t::iterator header_iterator;

		int	parseRequest(int fd);
        int executeMethod(Client* client, FdTable & fd_table);
		int	sendResponse(int fd);

    private:
        int methodGet(Client* client, FdTable & fd_table);
        int methodPost(Client* client, FdTable & fd_table);
        int methodDelete(Client* client, FdTable & fd_table);

		int		buildFile(Client* client, FdTable & fd_table);
		void	previewMethod();
		int		createFile(Client *client);
		int		insertFile(FdTable & fd_table);

		void	resetBuffer();

		void	generateHeaderString();
		void	generateResponse();

		std::string	ft_itoa(int	i) const;

    private:
		RequestParser::header_field_t  _header_fields;

		int					_oflag;
		AFdInfo::EventTypes	_file_event;
		File*				_file;
		RequestParser		_request_parser;

		std::string	_request;

		std::string	_response;
		std::string _http_version;
		std::string	_status_code;
		std::string	_status_phrase;
		std::string _header_string;
		std::string	_message_body;



};

