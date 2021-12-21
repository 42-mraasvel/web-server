#pragma once
#include "iHandler.hpp"
#include "fd/FdTable.hpp"
#include "fd/AFdInfo.hpp"
#include "parser/Request.hpp"
#include <string>

class File;

class FileHandler: public iHandler
{
	public:
		typedef SmartPointer<File> FilePointer;

	private:
		FileHandler(FileHandler const & rhs);
		FileHandler & operator=(FileHandler const & rhs);

	public:
		FileHandler();
		~FileHandler();

	/* Connection::readEvent */
	public:
		int		executeRequest(FdTable & fd_table, Request & request);
	private:
		int		createFile(FdTable & fd_table);
		void    setFileParameter();
		bool	isFileValid();
		bool		isFileExisted();
		bool		isFileAuthorized();
		bool		isUploadPathCreated();
		bool	openFile(FdTable & fd_table);
		int 	executeFile(Request & request);
		int 	executeGet();
		int 	executePost(Request & request);
		int 	executeDelete();
		void	updateFileEvent(FdTable & fd_table);

	/* update */
	public:
		void	update(std::string & response_body);
		void	exceptionEvent();
		bool	isComplete() const;
		bool	isError() const;
		int		redirectErrorPage(FdTable & fd_table, std::string const & file_path, int status_code);
		void	resetHandler(std::string const & file_path, int status_code);

	/* Connection::writeEvent */
	public:
		void	setSpecificHeaderField(HeaderField & header_field);
	private:
		void		setContentType(HeaderField & header_field) const;

	/* utility */
	public:
		void    	setAbsoluteFilePath(Request const & request);
		std::string	getAbsoluteFilePath() const;
		int			getStatusCode() const;
		bool		isFileError() const;
		bool		isFileComplete() const;
	private:	
		void    	deleteFile();
		void		markError(int status_code);

	private:
		Method::Type      		_method;
		int	            		_access_flag;
		int             		_open_flag;
		AFdInfo::EventTypes		_file_event;
		std::string     		_absolute_file_path;

		FilePointer		_file;
		int             _status_code;

		bool			_is_error;
		bool			_is_complete;
	   
};
