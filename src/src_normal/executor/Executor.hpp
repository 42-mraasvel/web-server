#pragma once
#include "parser/RequestParser.hpp"

//TODO: create response parser
class Executor
{
    public:
		typedef RequestParser::header_field_t::iterator header_iterator;

        int execute(RequestParser const & parser);
        std::string const & getResponse() const;

    private:
        int methodGet(RequestParser const & parser);
        int methodPost(RequestParser const & parser);
        int methodDelete(RequestParser const & parser);
        
		int generateHeaderString();
		int generateResponse();

    private:

        std::string _response;

		std::string _http_version;
		std::string	_status_code;
		std::string	_status_phrase;

		RequestParser::header_field_t  _header_fields;
		std::string _header_string;

		std::string	_message_body;

};

