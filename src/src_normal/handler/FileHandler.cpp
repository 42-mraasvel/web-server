#include "settings.hpp"
#include "FileHandler.hpp"
#include "fd/File.hpp"
#include <unistd.h>
#include <fcntl.h>

FileHandler::FileHandler(MethodType method): _method(method), _file(NULL), _response_complete(false) {}

void    FileHandler::setAbsoluteFilePath(std::string const & path)
{
	_absolute_file_path = path;
}

/*****************************/
/****** execute request ******/
/*****************************/

int	FileHandler::executeRequest(FdTable & fd_table, Request & request)
{
	if (createFile(fd_table) == ERR || executeFile(request) == ERR)
	{
		return ERR;
	}
	updateFileEvent(fd_table);
	return OK;
}


/*******************************************/
/****** execute request - create file ******/
/*******************************************/

int FileHandler::createFile(FdTable & fd_table)
{
	setFileParameter();

	if (!isFileReady() || !isFileOpened(fd_table))
	{
		return ERR;
	}
	return OK;

}

void	FileHandler::setFileParameter()
{
	switch(_method)
	{
		case GET:
			_access_flag = R_OK;
			_open_flag = O_RDONLY;
			_file_event = AFdInfo::READING;
			_status_code = 200;
			return ;
		case POST:
			_access_flag = W_OK;
			_open_flag = O_CREAT | O_WRONLY | O_APPEND;
			_file_event = AFdInfo::WRITING;
			_status_code = 201;
			return ;
		case DELETE:
			_access_flag = W_OK;
			_open_flag = O_WRONLY;
			_file_event = AFdInfo::WAITING;
			_status_code = 204;
			return ;
		case OTHER:
		default:
			return;
	}
}

bool	FileHandler::isFileReady()
{
	if (_method == GET || _method == DELETE)
	{
		return isFileExist() && isFileAuthorized();
	}
	if (_method == POST)
	{
		if (access(_absolute_file_path.c_str(), F_OK) == OK)
		{
			_status_code = 204; /* NO CONTENT */
			return isFileAuthorized();
		}
	}
	return true;
}

bool	FileHandler::isFileExist()
{
	if (access(_absolute_file_path.c_str(), F_OK) == ERR)
	{
		processError(404); /* NOTFOUND */
		return false;
	}
	return true;
}

bool	FileHandler::isFileAuthorized()
{
	if (access(_absolute_file_path.c_str(), _access_flag) == ERR)
	{
		processError(403); /* FORBIDDEN */
		return false;
	}
	return true;
}

bool	FileHandler::isFileOpened(FdTable & fd_table)
{
	int	file_fd = open(_absolute_file_path.c_str(), _open_flag, 0644);
	if (file_fd == ERR)
	{
		perror("open");
		processError(500); /* INTERNAL SERVER ERROR */
		return false;
	}
	printf(BLUE_BOLD "Open File:" RESET_COLOR " [%d]\n", file_fd);
	_file = new File(file_fd);
	fd_table.insertFd(_file);
	return true;
}

/*******************************************/
/****** execute request - execute file *****/
/*******************************************/

int FileHandler::executeFile(Request & request)
{
	switch (_method)
	{
		case GET:
			return executeGet();
		case POST:
			return executePost(request);
		case DELETE:
			return executeDelete();
		default:
			return OK;
	}
}

int	FileHandler::executeGet()
{
	return OK;
}

int	FileHandler::executePost(Request & request)
{
	_file->swapContent(request.message_body);
	return OK;
}

int	FileHandler::executeDelete()
{
	if (remove(_absolute_file_path.c_str()) == ERR)
	{
		perror("remove");
		processError(500); /* INTERNAL SERVER ERROR */
		return ERR;
	}
	printf(BLUE_BOLD "Delete File:" RESET_COLOR " [%s]\n", _absolute_file_path.c_str());
	_file->flag = AFdInfo::FILE_COMPLETE;
	return OK;
}

/******************************/
/****** generate response *****/
/******************************/

void	FileHandler::generateMessageBody(std::string & message_body, std::string const & effective_request_uri)
{
	switch (_method)
	{
		case GET:
			return generateMessageBodyGet(message_body);
		case POST:
			return generateMessageBodyPost(message_body, effective_request_uri);
		case DELETE:
			return generateMessageBodyDelete(message_body);
		default:
			return ;
	}
}

void	FileHandler::generateMessageBodyGet(std::string & message_body)
{
	message_body.append(_file->getContent());
	_file->clearContent();
}

void	FileHandler::generateMessageBodyPost(std::string & message_body, std::string const & effective_request_uri)
{
	if (_status_code == 201)
	{
		message_body = "New content created!\n" + effective_request_uri + "\n";
	}
	return ;
}

void	FileHandler::generateMessageBodyDelete(std::string & message_body)
{
	return ;
}

void	FileHandler::finishFile()
{
	if (!_response_complete
		&& _file
		&& _file->flag == AFdInfo::FILE_COMPLETE)
	{
		_response_complete = true;
		deleteFile();
	}
}

/*******************************/
/****** utility - private ******/
/*******************************/

void	FileHandler::updateFileEvent(FdTable & fd_table)
{
	_file->updateEvents(_file_event, fd_table);
}

void	FileHandler::deleteFile()
{
	if (_file)
	{
		_file->flag = AFdInfo::TO_ERASE;
		_file = NULL;		
	}
}

void	FileHandler::processError(int error_code)
{
	if (!_response_complete)
	{
		_status_code = error_code;
		_response_complete = true;
		deleteFile();
	}
}

/******************************/
/****** utility - public ******/
/******************************/

bool	FileHandler::getResponseComplete() const
{
	return _response_complete;
}

int	FileHandler::getStatusCode() const
{
	return _status_code;
}

bool	FileHandler::isChunked(std::string const & http_version) const
{
	if (!_response_complete
		&& http_version == "HTTP/1.1"
		&& _method == GET)
	{
		return _file
			&& _file->flag != AFdInfo::FILE_ERROR
			&& _file->getContent().size() >= BUFFER_SIZE;
	}
	return false;
}

bool	FileHandler::isFileReady() const
{
	return _file 
			&& (_file->flag == AFdInfo::FILE_COMPLETE
				|| _file->flag == AFdInfo::FILE_ERROR
				|| (_file->flag == AFdInfo::FILE_START
					&& !_file->getContent().empty()));
}

bool	FileHandler::isFileError()
{
	if (!_response_complete
		&& _file
		&& _file->flag == AFdInfo::FILE_ERROR)
	{
		processError(500); /* INTERNAL SERVER ERROR */
		return false;
	}
	return true;
}
