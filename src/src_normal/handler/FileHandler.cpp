#include "settings.hpp"
#include "FileHandler.hpp"
#include "fd/File.hpp"
#include "utility/status_codes.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>

FileHandler::FileHandler(MethodType method): _method(method), _file(NULL) {}

FileHandler::~FileHandler()
{
	if (_file)
	{
		deleteFile();
	}
}

/*****************************/
/****** execute request ******/
/*****************************/

int	FileHandler::executeRequest(FdTable & fd_table, Request & request)
{
	if (createFile(fd_table) == ERR)
	{
		return ERR;
	}
	if(executeFile(request) == ERR)
	{
		deleteFile();
		return ERR;
	}
	updateFileEvent(fd_table);
	return OK;
}

/*******************************************/
/****** execute request - create file ******/
/*******************************************/

int	FileHandler::createFile(FdTable & fd_table)
{
	setFileParameter();
	if (!isFileAccessible() || !openFile(fd_table))
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
			_status_code = StatusCode::STATUS_OK;
			return ;
		case POST:
			_access_flag = W_OK;
			_open_flag = O_CREAT | O_WRONLY | O_APPEND;
			_file_event = AFdInfo::WRITING;
			_status_code = StatusCode::CREATED;
			return ;
		case DELETE:
			_access_flag = W_OK;
			_open_flag = O_WRONLY;
			_file_event = AFdInfo::WAITING;
			_status_code = StatusCode::NO_CONTENT;
			return ;
		case OTHER:
		default:
			return;
	}
}

bool	FileHandler::isFileAccessible()
{
	if (_method == GET || _method == DELETE)
	{
		return isFileExist() && isFileAuthorized();
	}
	if (_method == POST)
	{
		if (access(_absolute_file_path.c_str(), F_OK) == OK)
		{
			_status_code = StatusCode::NO_CONTENT;
			return isFileAuthorized();
		}
	}
	return true;
}

bool	FileHandler::isFileExist()
{
	if (access(_absolute_file_path.c_str(), F_OK) == ERR)
	{
		_status_code = StatusCode::NOT_FOUND;
		return false;
	}
	return true;
}

bool	FileHandler::isFileAuthorized()
{
	if (access(_absolute_file_path.c_str(), _access_flag) == ERR)
	{
		_status_code = StatusCode::FORBIDDEN;
		return false;
	}
	return true;
}

bool	FileHandler::openFile(FdTable & fd_table)
{
	int	file_fd = open(_absolute_file_path.c_str(), _open_flag, 0644);
	if (file_fd == ERR)
	{
		perror("open");
		_status_code = StatusCode::INTERNAL_SERVER_ERROR;
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

int	FileHandler::executeFile(Request & request)
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
		_status_code = StatusCode::INTERNAL_SERVER_ERROR;
		return ERR;
	}
	printf(BLUE_BOLD "Delete File:" RESET_COLOR " [%s]\n", _absolute_file_path.c_str());
	_file->flag = AFdInfo::FILE_COMPLETE;
	return OK;
}

/************************************************/
/****** execute request - update file event *****/
/************************************************/

void	FileHandler::updateFileEvent(FdTable & fd_table)
{
	_file->updateEvents(_file_event, fd_table);
}

/********************************************/
/****** generate response - evaluateion *****/
/********************************************/

bool	FileHandler::evaluateExecutionError()
{
	if (isFileError())
	{
		deleteFile();
		return true;
	}
	return false;
}

bool	FileHandler::evaluateExecutionCompletion()
{
	if (isFileComplete())
	{
		deleteFile();
		return true;
	}
	return false;
}

/*********************************************/
/****** generate response - message body *****/
/*********************************************/

void	FileHandler::setMessageBody(std::string & message_body, std::string const & effective_request_uri)
{
	switch (_method)
	{
		case GET:
			return setMessageBodyGet(message_body);
		case POST:
			return setMessageBodyPost(message_body, effective_request_uri);
		case DELETE:
			return setMessageBodyDelete(message_body);
		default:
			return ;
	}
}

void	FileHandler::setMessageBodyGet(std::string & message_body)
{
	message_body.append(_file->getContent());
	_file->clearContent();
}

void	FileHandler::setMessageBodyPost(std::string & message_body, std::string const & effective_request_uri)
{
	if (_status_code == StatusCode::CREATED)
	{
		message_body = "New content created!\n" + effective_request_uri + "\n";
	}
	return ;
}

void	FileHandler::setMessageBodyDelete(std::string & message_body)
{
	return ;
}

/******************************/
/****** utility - public ******/
/******************************/

void    FileHandler::setAbsoluteFilePath(std::string const & path)
{
	_absolute_file_path = path;
}

int	FileHandler::getStatusCode() const
{
	return _status_code;
}

bool	FileHandler::isChunked(std::string const & http_version) const
{
	if (http_version == "HTTP/1.1"
		&& _method == GET)
	{
		return _file
			&& _file->flag != AFdInfo::FILE_ERROR
			&& _file->getContent().size() >= BUFFER_SIZE;
	}
	return false;
}

bool	FileHandler::isFileReadyForResponse() const
{
	return isFileError() || isFileComplete() || isFileReading();
}

bool	FileHandler::isFileError() const
{
	return _file && _file->flag == AFdInfo::FILE_ERROR;
}

bool	FileHandler::isFileComplete() const
{
	return _file && _file->flag == AFdInfo::FILE_COMPLETE;
}

bool	FileHandler::isFileReading() const
{
	return _file && _file->flag == AFdInfo::FILE_START && !_file->getContent().empty();
}

/*******************************/
/****** utility - private ******/
/*******************************/

void	FileHandler::deleteFile()
{
	_file->setToErase();
	_file = NULL;		
}
