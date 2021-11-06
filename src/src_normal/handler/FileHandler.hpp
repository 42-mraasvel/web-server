#pragma once
#include "fd/FdTable.hpp"
#include "fd/AFdInfo.hpp"
#include "parser/Request.hpp"
#include <string>

class File;

class FileHandler
{
	public:
		FileHandler(MethodType method);
		~FileHandler();

	/* execute request in read event */
	public:
		int		executeRequest(FdTable & fd_table, Request & request);
	private:
		int			createFile(FdTable & fd_table);
		void    	    setFileParameter();
		bool		    isFileAccessible();
		bool		    	isFileExist();
		bool		    	isFileAuthorized();
		bool			openFile(FdTable & fd_table);
		int 		executeFile(Request & request);
		int 			executeGet();
		int 			executePost(Request & request);
		int 			executeDelete();
		void		updateFileEvent(FdTable & fd_table);

	/* generate response in write event */
	public:
		bool	evaluateExecutionError();
		bool	evaluateExecutionCompletion();
		void	setMessageBody(std::string & message_body, std::string const & effective_request_uri);
	private:
		void		setMessageBodyGet(std::string & message_body);
		void		setMessageBodyPost(std::string & message_body, std::string const & effective_request_uri);
		void		setMessageBodyDelete(std::string & message_body);

	/* utility */
	public:
		void    setAbsoluteFilePath(std::string const & path);
		int		getStatusCode() const;
		bool	isChunked(std::string const & http_version) const;
		bool	isFileReadyForResponse() const;
		bool	isFileError() const;
		bool	isFileComplete() const;
		bool	isFileReading() const;
	private:
		void    deleteFile();

	private:
		MethodType      		_method;
		int	            		_access_flag;
		int             		_open_flag;
		AFdInfo::EventTypes		_file_event;
		std::string     		_absolute_file_path;

		File*           _file;
		int             _status_code;
	   

};
