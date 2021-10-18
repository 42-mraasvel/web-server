#pragma once
#include <string>
#include "parser/RequestParser.hpp"

class RequestParser;

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
