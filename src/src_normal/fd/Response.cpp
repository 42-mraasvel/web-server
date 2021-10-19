#include "Response.hpp"
#include "settings.hpp"
#include "fd/File.hpp"
#include <cstdlib>
#include <fcntl.h>


Response::Response(Request const & request): file(NULL), status(HEADER_PENDING), status_code(0)
{
	method = request.method;
	setHttpVersion(request.minor_version);
	previewMethod();
	generateAbsoluteTarget(request.target_resource);
}

Response::~Response()
{
	if (file)
	{
		delete file;
	}
}

int	Response::createFile()
{
	int	file_fd = open(absolute_target.c_str(), file_oflag, 0644);
	if (file_fd == ERR)
	{
		// TODO: add error handling (i.e. no file find)
		perror("open in method");
		return ERR;
	}
	printf(BLUE_BOLD "Open File:" RESET_COLOR " [%d]\n", file_fd);
	file = new File(file_fd);
	return OK;

}

void	Response::setHttpVersion(int minor_version)
{
	if (minor_version == 0)
	{
		http_version = "HTTP/1.0";
	}
	else
	{
		http_version = "HTTP/1.1";
	}
}

void Response::previewMethod()
{
	switch (method)
	{
		case GET:
			file_oflag = O_RDONLY;
			file_event = AFdInfo::READING;
			break;
		case POST:
			file_oflag = O_CREAT | O_WRONLY | O_APPEND;
			file_event = AFdInfo::WRITING;
			break;
		case DELETE:
			file_oflag = O_RDONLY;
			file_event = AFdInfo::READING;
			break; 
		default:
			break;
	}
}

void	Response::generateAbsoluteTarget(std::string const & target_resource)
{
	//TODO: resort to the correct Pathname based on default path from config (add Client* client)
	if (target_resource == "/")
	{
		absolute_target =  "./page_sample/index.html";
	}
	else
	{
		absolute_target =  "./page_sample" + target_resource;
	}
}

