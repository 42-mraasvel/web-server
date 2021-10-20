#include "Response.hpp"
#include "settings.hpp"
#include "fd/File.hpp"
#include "utility/utility.hpp"
#include <cstdlib>
#include <fcntl.h>


Response::Response(Request const & request): file(NULL), status(HEADER_PENDING), status_code(0)
{
	method = request.method;
	setHttpVersion(request.minor_version);
	previewMethod();
	generateAbsoluteTarget(request.target_resource);
}

Response::~Response() {}

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
			status_code = 200;
			break;
		case POST:
			file_oflag = O_CREAT | O_WRONLY | O_APPEND;
			file_event = AFdInfo::WRITING;
			status_code = 201;
			break;
		case DELETE:
			file_oflag = O_RDONLY;
			file_event = AFdInfo::READING;
			status_code = 202; //TODO: check if 204 no content
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

void	Response::deleteFile()
{
	file->flag = AFdInfo::TO_ERASE;
	file = NULL;
}

int	Response::generateResponse()
{
	if (status == HEADER_PENDING)
	{
		status = WITH_HEADER;
		switch (method)
		{
			case GET:
				responseGet();
				break;
			case POST:
				responsePost();
				break;
			case DELETE:
				responseDelete();
				break; 
			default:
				responseOther();
				break;
		}
	}
	header_fields["Host"] = "localhost";
	if (status == COMPLETE) // TODO: only when message_body is ready??
	{
		header_fields["Content-Length"] = WebservUtility::itoa(message_body.size());
	}
	setHeaderString(); //TODO: placeholder, to modify
	setResponseString(); //TODO: placeholder, to modify

	return OK;
}

void	Response::setHeaderString()
{
	for (header_iterator i = header_fields.begin(); i !=header_fields.end(); ++i)
	{
		header_string += (i->first + ": " + i->second + NEWLINE);
	}
}

void	Response::setResponseString()
{
	if (status == WITH_HEADER || status == COMPLETE)
	{
		string_to_send = http_version + " "
				+ WebservUtility::itoa(status_code) + " "
				+ WebservUtility::getStatusMessage(status_code)
				+ NEWLINE
				+ header_string
				+ NEWLINE
				+ message_body;
	}
	else if (status == MESSAGE_BODY_ONLY) //TODO: to sort out where mark flag MESSAGE_BODY_ONLy
	{
		string_to_send = message_body;
	}
}

int	Response::responseGet()
{
	message_body.append(file->getContent());
	file->clearContent();
	if (file->flag == AFdInfo::EVENT_COMPLETE)
	{
		status = COMPLETE;
		deleteFile();
	}
	return OK;
}

int	Response::responsePost()
{
	if (file->flag == AFdInfo::EVENT_COMPLETE)
	{
		status = COMPLETE;
		deleteFile();
	}
	return OK;
}

int	Response::responseDelete()
{
	if (file->flag == AFdInfo::EVENT_COMPLETE)
	{
		status = COMPLETE;
		deleteFile();
	}
	return OK;
}

int	Response::responseOther()
{
	return OK;
}
