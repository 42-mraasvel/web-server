#include "settings.hpp"
#include "FileHandler.hpp"
#include "fd/File.hpp"
#include "utility/status_codes.hpp"
#include "parser/HeaderField.hpp"
#include "MediaType.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>

FileHandler::FileHandler():
_file(NULL),
_status_code(0),
_is_error(false),
_is_complete(false)
{}

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
	_method = request.method;
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

std::string	FileHandler::resolvedRequestTarget(Request const & request)
{
	return request.config_info.resolved_file_path;
}

/*******************************************/
/****** execute request - create file ******/
/*******************************************/

int	FileHandler::createFile(FdTable & fd_table)
{
	setFileParameter();
	if (!isFileAuthorized() || !openFile(fd_table))
	{
		return ERR;
	}
	return OK;
}

void	FileHandler::setFileParameter()
{
	switch(_method)
	{
		case Method::GET:
			_access_flag = R_OK;
			_open_flag = O_RDONLY;
			_file_event = AFdInfo::READING;
			if (_status_code == 0)
			{
				_status_code = StatusCode::STATUS_OK;
			}
			return ;
		case Method::POST:
			_access_flag = W_OK;
			_open_flag = O_CREAT | O_WRONLY | O_APPEND;
			_file_event = AFdInfo::WRITING;
			if (_status_code == 0)
			{
				_status_code = StatusCode::CREATED;
			}
			return ;
		case Method::DELETE:
			_access_flag = W_OK;
			_open_flag = O_WRONLY;
			_file_event = AFdInfo::WAITING;
			if (_status_code == 0)
			{
				_status_code = StatusCode::NO_CONTENT;
			}
			return ;
		case Method::OTHER:
		default:
			return;
	}
}

bool	FileHandler::isFileAuthorized()
{
	if (access(_absolute_file_path.c_str(), _access_flag) == ERR)
	{
		markError(StatusCode::FORBIDDEN);
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
		markError(StatusCode::INTERNAL_SERVER_ERROR);
		return false;
	}
	printf(BLUE_BOLD "Open File:" RESET_COLOR " %s: [%d]\n", _absolute_file_path.c_str(), file_fd);
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
		case Method::GET:
			return executeGet();
		case Method::POST:
			return executePost(request);
		case Method::DELETE:
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
	_file->appendContent(request.message_body);
	return OK;
}

int	FileHandler::executeDelete()
{
	if (remove(_absolute_file_path.c_str()) == ERR)
	{
		perror("remove");
		markError(StatusCode::INTERNAL_SERVER_ERROR);
		return ERR;
	}
	printf(BLUE_BOLD "Delete File:" RESET_COLOR " [%s]\n", _absolute_file_path.c_str());
	_file->setFlag(AFdInfo::COMPLETE);
	return OK;
}

/************************************************/
/****** execute request - update file event *****/
/************************************************/

void	FileHandler::updateFileEvent(FdTable & fd_table)
{
	_file->updateEvents(_file_event, fd_table);
}

/*******************/
/****** update *****/
/*******************/

void	FileHandler::update()
{
	if (isError())
	{
		return ;
	}

	if (isFileError())
	{
		deleteFile();
		_status_code = StatusCode::INTERNAL_SERVER_ERROR;
		_is_error = true;
		return ;
	}

	_file->appendContent(_message_body);

	if (isFileComplete())
	{
		deleteFile();
		_is_complete = true;
		return ;
	}
}

int	FileHandler::redirectErrorPage(FdTable & fd_table, std::string const & file_path, int status_code)
{
	_absolute_file_path = file_path;
	_status_code = status_code;
	Request	error_page_request;
	error_page_request.method = Method::GET;
	return executeRequest(fd_table, error_page_request);
}

/**************************************/
/****** update - set message body *****/
/**************************************/

void	FileHandler::setMessageBody(std::string & message_body)
{
	switch (_method)
	{
		case Method::GET:
			return setMessageBodyGet(message_body);
		case Method::POST:
			return setMessageBodyPost();
		case Method::DELETE:
			return setMessageBodyDelete();
		default:
			return ;
	}
}

void	FileHandler::setMessageBodyGet(std::string & message_body)
{
	if (message_body.size() == 0)
	{
		message_body.swap(_message_body);
	}
	else
	{
		message_body.append(_message_body);
		_message_body.clear();
	}
}

void	FileHandler::setMessageBodyPost()
{
	return ;
}

void	FileHandler::setMessageBodyDelete()
{
	return ;
}

/******************************/
/****** set header field ******/
/******************************/

void    FileHandler::setSpecificHeaderField(HeaderField & header_field)
{
	if (!header_field.contains("Content-Type"))
	{
		setContentType(header_field);
	}
}

void	FileHandler::setContentType(HeaderField & header_field) const
{
	if (_method == Method::GET && !_absolute_file_path.empty())
	{
		header_field["Content-Type"] = MediaType::getMediaType(_absolute_file_path);
		return ;
	}
	if (!_message_body.empty())
	{
		header_field["Content-Type"] = "text/plain;charset=UTF-8";
	}
}

/******************************/
/****** utility - public ******/
/******************************/

bool	FileHandler::isComplete() const
{
	return _is_complete;
}

bool	FileHandler::isError() const
{
	return _is_error;
}

void    FileHandler::setAbsoluteFilePath(std::string const & path)
{
	_absolute_file_path = path;
}

std::string    FileHandler::getAbsoluteFilePath() const
{
	return _absolute_file_path;
}

int	FileHandler::getStatusCode() const
{
	return _status_code;
}

bool	FileHandler::isReadyToWrite() const
{
	return isFileError() || isFileComplete();
}

bool	FileHandler::isFileError() const
{
	return _file && _file->getFlag() == AFdInfo::ERROR;
}

bool	FileHandler::isFileComplete() const
{
	return _file && _file->getFlag() == AFdInfo::COMPLETE;
}

bool	FileHandler::isFileReading() const
{
	return _file && _file->getFlag() == AFdInfo::START && !_file->getContent().empty();
}

/*******************************/
/****** utility - private ******/
/*******************************/

void	FileHandler::deleteFile()
{
	_file->setToErase();
	_file = NULL;		
}

void	FileHandler::markError(int status_code)
{
	_status_code = status_code;
	_is_error = true;
}
