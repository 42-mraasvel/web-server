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

	/* execute request in read event */
	public:
		bool	executeRequest(FdTable & fd_table, Request & request);
	private:
		bool		createFile(FdTable & fd_table);
		void    	    setFileParameter();
		bool		    isFileAccessible();
		bool		    	isFileExist();
		bool		    	isFileAuthorized();
		bool			openFile(FdTable & fd_table);
		bool 		executeFile(Request & request);
		bool 			executeGet();
		bool 			executePost(Request & request);
		bool 			executeDelete();
		void		updateFileEvent(FdTable & fd_table);

	/* generate response in write event */
	public:
		bool	isFileEventError();
		void	generateMessageBody(std::string & message_body, std::string const & effective_request_uri);
		void	finish();
	private:
		void		generateMessageBodyGet(std::string & message_body);
		void		generateMessageBodyPost(std::string & message_body, std::string const & effective_request_uri);
		void		generateMessageBodyDelete(std::string & message_body);

	/* utility */
	private:
		void    deleteFile();
		void    processError(int error_code);

	public:
		void    setAbsoluteFilePath(std::string const & path);
		bool	isChunked(std::string const & http_version) const;
		bool	isFileReadyForResponse() const;
		int		getStatusCode() const;
		void	clean();

	private:
		MethodType      		_method;
		int	            		_access_flag;
		int             		_open_flag;
		AFdInfo::EventTypes		_file_event;
		std::string     		_absolute_file_path;

		File*           _file;
		int             _status_code;
	   

};
