#include "Handler.hpp"
#include "settings.hpp"
#include "fd/Client.hpp"
#include "fd/File.hpp"
#include <fcntl.h>
#include <cstdlib>
#include <sstream>
#include <poll.h>
#include <sys/socket.h>


Handler::Handler(): _file(NULL), _client(NULL) {}

void	Handler::setClient(Client* client)
{
	_client = client;
}

/****************************/
/****** processRequest ******/
/****************************/

int	Handler::processRequest(FdTable & fd_table)
{
	previewMethod();

	// TODO: (from Maarten) check for CONT_READING or NOT
	if (parseRequest() == ERR)
	{
		return ERR;
	}
	if (executeMethod(fd_table) == ERR)
	{
		return ERR;
	}
	_client->updateEvents(AFdInfo::WAITING, fd_table);

	return OK;
}

void Handler::previewMethod()
{
	switch (_request_parser.getMethod())
	{
		case RequestParser::GET:
			_oflag = O_RDONLY;
			_file_event = AFdInfo::READING;
			break;
		case RequestParser::POST:
			_oflag = O_CREAT | O_WRONLY | O_APPEND;
			_file_event = AFdInfo::WRITING;
			break;
		case RequestParser::DELETE:
			_oflag = O_RDONLY;
			_file_event = AFdInfo::READING;
			break; 
		default:
			break;
	}
}

//TODO: Make recv work with multiple iterations, so each iter can loop over request
int	Handler::parseRequest()
{
	//TODO: CHECK MAXLEN
	_request.resize(BUFFER_SIZE, '\0');
	ssize_t ret = recv(_client->getFd(), &_request[0], BUFFER_SIZE, 0);
	if (ret == ERR)
	{
		perror("Recv");
		return (ERR);
	}
	else if (ret == 0)
	{
		_client->flag = AFdInfo::TO_ERASE;
		return ERR;
	}
	_request.resize(ret);
	printf("Request size: %lu, Bytes read: %ld\n", _request.size(), ret);

	//TODO: check for continue reading and change the return value
	if(_request_parser.parseHeader(_request) == RequestParser::BAD_REQUEST)
	{
		return (ERR);
	}

	generateAbsoluteTarget();

	return OK;
}

void	Handler::generateAbsoluteTarget()
{
	//TODO: resort to the correct Pathname based on default path from config (add Client* client)
	if (_request_parser.getTargetResource() == "/")
	{
		_absolute_target =  "./page_sample/index.html";
	}
	else
	{
		_absolute_target =  "./page_sample" + _request_parser.getTargetResource();
	}
}

//TODO: create response with error status
int Handler::executeMethod(FdTable & fd_table)
{
	if (createFile() == ERR
		|| insertFile(fd_table) == ERR)
	{
		return ERR;
	}

	switch (_request_parser.getMethod())
	{
		case RequestParser::GET:
			methodGet();
			break;
		case RequestParser::POST:
			methodPost();
			break;
		case RequestParser::DELETE:
			methodDelete();
			break; 
		default:
			methodOther();
			break;
	}

	resetBuffer(); //for _request
	return OK;
}

int	Handler::createFile()
{

	int	file_fd = open(_absolute_target.c_str(), _oflag, 0644);
	if (file_fd == ERR)
	{
		// TODO: add error handling (i.e. no file find)
		perror("open in method");
		return ERR;
	}

	printf(BLUE_BOLD "Open File:" RESET_COLOR " [%d]\n", file_fd);

	_file = new File(_client, file_fd);

	return OK;

}

int	Handler::insertFile(FdTable & fd_table)
{
	int	file_index = fd_table.size();
	if (fd_table.insertFd(_file) == ERR)
	{
		return ERR;
	}
	_file->updateEvents(_file_event, fd_table);

	return OK;
}

int	Handler::methodGet()
{
	return OK;
}

int	Handler::methodPost()
{
	_file->setContent(_request_parser.getMessageBody());

	return OK;
}

int	Handler::methodDelete()
{

	if (remove(_request_parser.getTargetResource().c_str()) == ERR)
	{
		perror("remove in methodDelete");
		return ERR;
	}
	printf(BLUE_BOLD "Delete File:" RESET_COLOR " [%s]\n", _request_parser.getTargetResource().c_str());

	return OK;
}

int	Handler::methodOther()
{
	return OK;
}


//TODO: retain information about the next request if present
void	Handler::resetBuffer()
{
	_request.clear();
}

/***************************/
/****** sendResponse ******/
/**************************/

int	Handler::sendResponse(FdTable & fd_table)
{
	generateResponse();

	_file->flag = AFdInfo::TO_ERASE;
	_file = NULL;

	if (send(_client->getFd(), _response.c_str(), _response.size(), 0) == ERR)
	{
		perror("send");
		return ERR;
	}

	_client->updateEvents(AFdInfo::READING, fd_table);

	return OK;
}

void	Handler::generateResponse()
{
	switch (_request_parser.getMethod())
	{
		case RequestParser::GET:
			responseGet();
			break;
		case RequestParser::POST:
			responsePost();
			break;
		case RequestParser::DELETE:
			responseDelete();
			break; 
		default:
			responseOther();
			break;
	}
	_http_version = "HTTP/1.1";
	_header_fields["Host"] = "localhost";
	_header_fields["Content-Length"] = ft_itoa(_message_body.size());

	convertHeaderString();
	
	_response = _http_version + ' '
				+ _status_code + ' '
				+ _status_phrase + NEWLINE
				+ _header_string + NEWLINE
				+ _message_body;
}

int	Handler::responseGet()
{
	_status_code = "200";
	_status_phrase = "OK";
	_message_body = _file->getContent();
	return OK;
}

int	Handler::responsePost()
{
	_status_code = "201";
	_status_phrase = "Created";
	return OK;
}

int	Handler::responseDelete()
{
	//TODO: check if 204 no content
	_status_code = "202";
	_status_phrase = "Accepted";
	return OK;
}

int	Handler::responseOther()
{
	return OK;
}


void	Handler::convertHeaderString()
{
	for (header_iterator i = _header_fields.begin(); i != _header_fields.end(); ++i)
	{
		_header_string += (i->first + ": " + i->second + NEWLINE);
	}
}

/***************************/
/********* Utility *********/
/***************************/

std::string	Handler::ft_itoa(int i) const
{
	std::stringstream	ss;
	ss << i;
	return ss.str();
}
