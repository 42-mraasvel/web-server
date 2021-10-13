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

		enum RequestParser::MethodType	getMethod() const;

    private:
        int methodGet(Client* client, FdTable & fd_table);
        int methodPost(Client* client, FdTable & fd_table);
        int methodDelete(Client* client, FdTable & fd_table);
        
		int	setFile(Client* client, int file_fd, FdTable & fd_table, AFdInfo::EventTypes events);

		void	generateHeaderString();
		void	generateResponse();

		std::string	ft_itoa(int	i) const;

		void	resetBuffer();

    private:
		std::string _http_version;
		std::string	_status_code;
		std::string	_status_phrase;
		std::string _header_string;
		std::string	_message_body;
		std::string	_response;

		std::string	_request;

		RequestParser::header_field_t  _header_fields;

		File*			_file;
		RequestParser	_request_parser;

};

