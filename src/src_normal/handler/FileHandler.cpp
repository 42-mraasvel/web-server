#include "settings.hpp"
#include "FileHandler.hpp"
#include "fd/File.hpp"
#include "utility/status_codes.hpp"
#include "utility/utility.hpp"
#include "parser/HeaderField.hpp"
#include "MediaType.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <errno.h>

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
	if (executeFile(request) == ERR)
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
	if (!isFileValid() || !openFile(fd_table))
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
			_open_flag = O_CREAT | O_WRONLY | O_TRUNC;
			_file_event = AFdInfo::WRITING;
			if (_status_code == 0)
			{
				_status_code = StatusCode::NO_CONTENT;
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
		default:
			return;
	}
}

bool	FileHandler::isFileValid()
{
	if (_method != Method::POST)
	{
		return isFileExisted() && isFileAuthorized();
	}
	else
	{
		return isUploadPathCreated();
	}
}

bool	FileHandler::isFileExisted()
{
	if (!WebservUtility::isFileExisted(_absolute_file_path))
	{
		if (errno == EACCES)
		{
			markError(StatusCode::FORBIDDEN);
		}
		else
		{
			markError(StatusCode::NOT_FOUND);
		}
		return false;
	}
	return true;
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

static bool	isUploadPathValid(std::string const & path)
{
	return !(path.empty() || path[path.size() - 1] == '/' || path[0] != '/' || path.find("//") != std::string::npos);
}

/* upload path cannot end with "/" or contain "//" */
bool	FileHandler::isUploadPathCreated()
{
	if (!isUploadPathValid(_absolute_file_path))
	{
		markError(StatusCode::BAD_REQUEST);
		return false;
	}
	if (WebservUtility::createDirectories(_absolute_file_path) == ERR)
	{
		if (errno == EACCES)
		{
			markError(StatusCode::FORBIDDEN);
		}
		else
		{
			syscallError(_FUNC_ERR("create upload path"));
			markError(StatusCode::INTERNAL_SERVER_ERROR);
		}
		return false;
	}
	if (!WebservUtility::isFileExisted(_absolute_file_path))
	{
		_status_code = StatusCode::CREATED;
	}
	else
	{
		return isFileAuthorized();
	}
	return true;
}

bool	FileHandler::openFile(FdTable & fd_table)
{
	int	file_fd = open(_absolute_file_path.c_str(), _open_flag, 0644);
	if (file_fd == ERR)
	{
		syscallError(_FUNC_ERR("open"));
		markError(StatusCode::INTERNAL_SERVER_ERROR);
		return false;
	}
	if (WebservUtility::makeNonBlocking(file_fd) == ERR)
	{
		syscallError(_FUNC_ERR("fcntl"));
		close(file_fd);
		markError(StatusCode::INTERNAL_SERVER_ERROR);
		return false;
	}
	PRINT_DEBUG << BLUE_BOLD "Open File" RESET_COLOR ": " << _absolute_file_path << ": [" << file_fd << "]" << std::endl;
	try
	{
		_file = FilePointer(new File(file_fd));
	}
	catch (...)
	{
		close(file_fd);
		throw;
	}
	fd_table.insertFd(SmartPointer<AFdInfo>(_file));
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
	_file->appendFromContent(request.message_body);
	return OK;
}

int	FileHandler::executeDelete()
{
	if (remove(_absolute_file_path.c_str()) == ERR)
	{
		syscallError(_FUNC_ERR("remove"));
		markError(StatusCode::INTERNAL_SERVER_ERROR);
		return ERR;
	}
	PRINT_DEBUG << BLUE_BOLD "Delete File" RESET_COLOR ": [" << _absolute_file_path << "]" << std::endl;
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

void	FileHandler::update(std::string & response_body)
{
	if (isError())
	{
		return ;
	}

	if (isFileError())
	{
		markError(StatusCode::INTERNAL_SERVER_ERROR);
		return ;
	}

	if (_method == Method::GET && _file
		&& (_file->getFlag() == AFdInfo::START || _file->getFlag() == AFdInfo::COMPLETE))
	{
		_file->appendToContent(response_body);
	}

	if (isFileComplete())
	{
		deleteFile();
		_is_complete = true;
		return ;
	}
}

void	FileHandler::exceptionEvent()
{
	markError(StatusCode::INTERNAL_SERVER_ERROR);
	PRINT_ERR << "FileHandler: exceptionEvent" << std::endl;
}

int	FileHandler::redirectErrorPage(FdTable & fd_table, std::string const & file_path, int status_code)
{
	resetHandler(file_path, status_code);
	Request	error_page_request;
	error_page_request.method = Method::GET;
	return executeRequest(fd_table, error_page_request);
}

void	FileHandler::resetHandler(std::string const & file_path, int status_code)
{
	_absolute_file_path = file_path;
	_status_code = status_code;
	_is_error = false;
	_is_complete = false;
	_file = NULL;
}

/******************************/
/****** set header field ******/
/******************************/

void    FileHandler::setSpecificHeaderField(HeaderField & header_field, bool content_type_fixed)
{
	if (!content_type_fixed)
	{
		setContentType(header_field);
	}
}

void	FileHandler::setContentType(HeaderField & header_field) const
{
	if (_method == Method::GET && !_absolute_file_path.empty())
	{
		header_field["Content-Type"] = MediaType::getMediaType(_absolute_file_path);
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

void    FileHandler::setAbsoluteFilePath(Request const & request)
{
	if (request.method == Method::POST
		&& !request.config_info.resolved_location->_upload_store.empty())
	{
		_absolute_file_path = request.config_info.resolved_location->_upload_store + request.config_info.resolved_target;
	}
	else
	{
		_absolute_file_path = request.config_info.resolved_file_path;
	}
}

std::string    FileHandler::getAbsoluteFilePath() const
{
	return _absolute_file_path;
}

int	FileHandler::getStatusCode() const
{
	return _status_code;
}

bool	FileHandler::isFileError() const
{
	return _file && _file->getFlag() == AFdInfo::ERROR;
}

bool	FileHandler::isFileComplete() const
{
	return _file && _file->getFlag() == AFdInfo::COMPLETE;
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
	_absolute_file_path.clear();
	if (_file)
	{
		deleteFile();
	}
}
