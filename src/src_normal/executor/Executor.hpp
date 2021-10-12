#pragma once
#include "parser/RequestParser.hpp"
#include "fd/FdTable.hpp"

class Client;

class File;

//TODO: create response parser
class Executor
{
    public:
		typedef RequestParser::header_field_t::iterator header_iterator;

        int execute(Client* client, FdTable & fd_table, RequestParser const &parser);
        std::string const & getResponse() const;
		void	generateResponse(File* file);

    private:
        int methodGet(Client* client, FdTable & fd_table, RequestParser const &parser);
        int methodPost(RequestParser const & parser);
        int methodDelete(RequestParser const & parser);
        
		void	generateHeaderString();

		std::string	ft_itoa(int	i) const;

    private:

		std::string	_response;

		std::string _http_version;
		std::string	_status_code;
		std::string	_status_phrase;

		RequestParser::header_field_t  _header_fields;
		std::string _header_string;

		std::string	_message_body;

};

