#include "Executor.hpp"
#include "settings.hpp"
#include "fd/File.hpp"

//TODO: ERROR
//TODO: create response with error status
//TODO: implement actual responses
int Executor::execute(RequestParser const &parser)
{

	switch (parser.getMethod())
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
			break;
	}
	return genericResponse(); 
	return (OK);
}


int	Executor::methodGet()
{
	File	new_file;
	
	return printf("Get method called\n");
}

int	Executor::methodPost()
{
	return printf("Post method called\n");
}
int	Executor::methodDelete()
{
	return printf("Delete method called\n");
}

int	Executor::genericResponse()
{
	_response =
	"HTTP/1.1 200 OK\r\n"
	"Host: localhost\r\n"
	"Content-Length: 12\r\n\r\n"
	"Hello, World!";
	return OK;
}

std::string const & Executor::getResponse() const
{
	return _response;
}
