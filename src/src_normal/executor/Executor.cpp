#include "Executor.hpp"
#include "settings.hpp"
#include "fd/File.hpp"
#include <fcntl.h>
#include <cstdlib>

//TODO:: error checking for each function


//TODO: ERROR
//TODO: create response with error status
//TODO: implement actual responses
int Executor::execute(RequestParser const &parser)
{

	switch (parser.getMethod())
	{
		case RequestParser::GET:
			methodGet(parser);
			break;
		case RequestParser::POST:
			methodPost(parser);
			break;
		case RequestParser::DELETE:
			methodDelete(parser);
			break; 
		default:
			break;
	}
	return generateResponse(); 
	return (OK);
}


int	Executor::methodGet(RequestParser const & parser)
{
	_http_version = "HTTP/1.1";
	_status_code = "200";
	_status_phrase = "OK";
	
	_header_fields.insert(std::pair<std::string, std::string>("Host", "localhost"));
	_header_fields.insert(std::pair<std::string, std::string>("Content-Length", "12"));

	_message_body = "Hello, Lemon!";

//	int	file_fd = open(parser.getTargetResource().c_str(), O_RDONLY);
	// TODO: check open error.
//	File	file(????,file_fd);
	
	return OK;
}

int	Executor::methodPost(RequestParser const & parser)
{
	return OK;
}
int	Executor::methodDelete(RequestParser const & parser)
{
	return OK;
}

int	Executor::generateHeaderString()
{
	for (header_iterator i = _header_fields.begin(); i != _header_fields.end(); ++i)
	{
		_header_string += (i->first + ": " + i->second + NEWLINE);
	}
	return OK;
}

int	Executor::generateResponse()
{
/*
	_response =
		"HTTP/1.1 200 OK\r\n"
		"Host: localhost\r\n"
		"Content-Length: 12\r\n\r\n"
		"Hello, World!";
*/
	
	if (generateHeaderString() == ERR)
		return ERR;
	_response = _http_version + ' '
				+ _status_code + ' '
				+ _status_phrase + NEWLINE
				+ _header_string + NEWLINE
				+ _message_body;
	return OK;
}

std::string const & Executor::getResponse() const
{
	return _response;
}
