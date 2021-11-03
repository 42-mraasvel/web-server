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

	/* execute request in read event*/
	public:
		int		executeRequest(FdTable & fd_table, Request & request);
	private:
		int			createFile(FdTable & fd_table);
		void    	    setFileParameter();
		bool		    isFileReady();
		bool		    	isFileExist();
		bool		    	isFileAuthorized();
		bool			isFileOpened(FdTable & fd_table);
		int 		executeFile(Request & request);
		int 			executeGet();
		int 			executePost(Request & request);
		int 			executeDelete();

	/* generate response in write event*/
	public:
		void		generateMessageBody(std::string & message_body, std::string const & effective_request_uri);
	private:
		void			generateMessageBodyGet(std::string & message_body);
		void			generateMessageBodyPost(std::string & message_body, std::string const & effective_request_uri);
		void			generateMessageBodyDelete(std::string & message_body);

	private:
		void	updateFileEvent(FdTable & fd_table);
		void    deleteFile();
		void    processError(int error_code);

	public:
		void    setAbsoluteFilePath(std::string const & path);
		void	finishFile();
		bool	isChunked(std::string const & http_version) const;
		bool	isFileReady() const;
		bool	isFileError();
		bool	getResponseComplete() const;
		int		getStatusCode() const;

	private:
		MethodType      		_method;
		int	            		_access_flag;
		int             		_open_flag;
		AFdInfo::EventTypes		_file_event;
		std::string     		_absolute_file_path;

		File*           _file;
 		bool			_response_complete;
		int             _status_code;
	   

};
