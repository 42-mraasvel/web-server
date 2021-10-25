#pragma once
#include <string>
#include "parser/RequestParser.hpp"
#include "AFdInfo.hpp"
#include "parser/Request.hpp"

class File;

class Response
{
	public:
		enum Status
		{
			START,
			HEADER_COMPLETE,
			COMPLETE
		};

		Response();
		~Response();

	/* Client::readEvent() */
	public:
		void	scanRequest(Request const & request);
	private:
		void		setHttpVersion(int minor_version);
		void		previewMethod();
		void		generateAbsoluteTarget(std::string const & target_resourse);
		bool		isRequestError(Request const & request);
		bool			checkBadRequest(Request::RequestStatus status, int request_code);
		bool			checkHttpVersion(int http_major_version);
		bool			checkMethod();
		bool			checkExpectation(Request const & request);
		bool			checkContentLength(Request const & request);
		void		immediateResponse(Request const & request);

	public:
		void	executeRequest(FdTable & fd_table, Request & request);
	private:
		int			createFile(FdTable & fd_table);
        int 		executeMethod(Request & request);
        int 			methodGet();
        int 			methodPost(Request & request);
        int 			methodDelete();

	/* Client::writeEvent() */
	public:
		void	generateResponse();
	private:
		void		responseGet();
		void		responsePost();
		void		responseDelete();
		void		setHeaderString();
		void		setResponseString();
	
	/* utility */
	public:
		typedef RequestParser::header_field_t::const_iterator header_iterator;
		Status				getStatus() const;
		int					getStatusCode() const;
		std::string const &	getString() const;
		void				clearString();
		void				deleteFile();
		void				updateFileEvent(FdTable & fd_table);
		bool				isFileReady() const;

	private:
		void	processError(int error_code);
		void		generateErrorPage();

	private:
		MethodType			_method;
		Status				_status;
		File*				_file;
		std::string			_string_to_send;

		std::string			_absolute_target;
		int					_file_oflag;
		AFdInfo::EventTypes	_file_event;
		RequestParser::header_field_t  _header_fields;

		std::string 		_http_version;
		int					_status_code;
		std::string 		_header_string;
		std::string			_message_body;
};
