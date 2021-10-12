#include "Handler.hpp"
#include "settings.hpp"
#include "fd/Client.hpp"
#include "fd/File.hpp"
#include <fcntl.h>
#include <cstdlib>
#include <sstream>
#include <poll.h>
#include <sys/socket.h>

int	Handler::parseRequest(int fd)
{
	//TODO: CHECK MAXLEN
	if (_request.size() + BUFFER_SIZE >= _request.capacity())
	{
		_request.reserve(std::max((size_t)BUFFER_SIZE, _request.capacity() * 2));
	}
	int ret = recv(fd, &_request[_request.size()], BUFFER_SIZE, 0);
	if (ret == ERR)
	{
		perror("Recv");
		return (ERR);
	}
	printf("len read: %d\n",printf("%s\n", _request.c_str()));

	//TODO: Parse Header
	if(_request_parser.parseHeader(_request) == ERR)
	{
		return (ERR);
	}

	return OK;

}

//TODO: ERROR
//TODO: create response with error status
//TODO: implement actual responses
int Handler::executeMethod(Client* client, FdTable & fd_table)
{
	// TODO: add error handling from Parser (Status != 200)

	switch (_request_parser.getMethod())
	{
		case RequestParser::GET:
			methodGet(client, fd_table);
			break;
		case RequestParser::POST:
			methodPost(client, fd_table);
			break;
		case RequestParser::DELETE:
			methodDelete(client, fd_table);
			break; 
		default:
			break;
	}
	resetBuffer(); //for _request
	return OK;
}


int	Handler::methodGet(Client* client, FdTable & fd_table)
{
	int	file_fd = open(_request_parser.getTargetResource().c_str(), O_RDONLY);
	if (file_fd == ERR)
	{
		// TODO: add error handling (i.e. no file find)
		perror("open in methodGet");
		return ERR;
	}
	printf(BLUE_BOLD "Open File:" RESET_COLOR " [%d]\n", file_fd);

	if (setFile(client, file_fd, fd_table, AFdInfo::READING) == ERR)
	{
		return ERR;
	}

	return OK;
}

int	Handler::methodPost(Client* client, FdTable & fd_table)
{
	int	file_fd = open(_request_parser.getTargetResource().c_str(), O_CREAT | O_WRONLY);
	if (file_fd == ERR)
	{
		perror("open in methodPost");
		return ERR;
	}
	printf(BLUE_BOLD "Open File:" RESET_COLOR " [%d]\n", file_fd);

	if (setFile(client, file_fd, fd_table, AFdInfo::WRITING) == ERR)
	{
		return ERR;
	}

	return OK;
}

int	Handler::methodDelete(Client* client, FdTable & fd_table)
{
	return OK;
}

int	Handler::setFile(Client* client, int file_fd, FdTable & fd_table, AFdInfo::EventTypes events)
{
	_file = new File(client, file_fd);
	int	file_index = fd_table.size();
	if (fd_table.insertFd(_file) == ERR)
	{
		return ERR;
	}
	_file->updateEvents(events, fd_table);

	return OK;
}


void	Handler::generateHeaderString()
{
	for (header_iterator i = _header_fields.begin(); i != _header_fields.end(); ++i)
	{
		_header_string += (i->first + ": " + i->second + NEWLINE);
	}
}

void	Handler::generateResponse()
{

	_message_body = _file->getContent();
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

	_file->flag = AFdInfo::TO_ERASE;

}

std::string	Handler::ft_itoa(int i) const
{
	std::stringstream	ss;
	ss << i;
	return ss.str();
}

//TODO: retain information about the next request if present
void	Handler::resetBuffer()
{
	_request.clear();
}

int	Handler::sendResponse(int fd)
{
	generateResponse();

	if (send(fd, _response.c_str(), _response.size(), 0) == ERR)
	{
		perror("send");
		return ERR;
	}

	return OK;
}
