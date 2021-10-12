#include "Executor.hpp"
#include "settings.hpp"
#include "fd/Client.hpp"
#include "fd/File.hpp"
#include <fcntl.h>
#include <cstdlib>
#include <sstream>
#include <poll.h>

//TODO:: error checking for each function


//TODO: ERROR
//TODO: create response with error status
//TODO: implement actual responses
int Executor::execute(Client* client, FdTable & fd_table, RequestParser const &parser)
{

	switch (parser.getMethod())
	{
		case RequestParser::GET:
			methodGet(client, fd_table, parser);
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
	return OK;
}


int	Executor::methodGet(Client* client, FdTable & fd_table, RequestParser const & parser)
{
	// TODO: add error handling from Parser (Status != 200)

	// TODO: replace file with parser.getTargetResource().c_str().
	int	file_fd = open("./page_sample/Anne", O_RDONLY);
	if (file_fd == ERR)
	{
		perror("open");
		return ERR;
	}
	printf(BLUE_BOLD "Open File:" RESET_COLOR " [%d]\n", file_fd);

	if (client->setFile(file_fd, fd_table) == ERR)
	{
		return ERR;
	}

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

void	Executor::generateHeaderString()
{
	for (header_iterator i = _header_fields.begin(); i != _header_fields.end(); ++i)
	{
		_header_string += (i->first + ": " + i->second + NEWLINE);
	}
}

void	Executor::generateResponse(File* file)
{

	_message_body = file->getContent();
	_http_version = "HTTP/1.1";
	_status_code = "200";
	_status_phrase = "OK";
//	_message_body = "Hello, Lemon! lalala";
	_header_fields.insert(std::pair<std::string, std::string>("Host", "localhost"));
	_header_fields.insert(std::pair<std::string, std::string>("Content-Length", ft_itoa(_message_body.size())));

	generateHeaderString();
	
	_response = _http_version + ' '
				+ _status_code + ' '
				+ _status_phrase + NEWLINE
				+ _header_string + NEWLINE
				+ _message_body;
}

std::string const & Executor::getResponse() const
{
	return _response;
}

std::string	Executor::ft_itoa(int i) const
{
	std::stringstream	ss;
	ss << i;
	return ss.str();
}

