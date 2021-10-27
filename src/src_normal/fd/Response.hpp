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
		void		continueResponse(Request const & request);

	public:
		void	executeRequest(FdTable & fd_table, Request & request);
	private:
		int			createFile(FdTable & fd_table);
        int 		executeMethod(Request & request);
        int 			executeGet();
        int 			executePost(Request & request);
        int 			executeDelete();

	/* Client::writeEvent() */
	public:
		void	prepareToWrite();
	private:
		void		checkFileError();
		void		defineEncoding();

	public:
		void	generateResponse();
	private:
		void		responseMethod();
		void			responseGet();
		void			responsePost();
		void			responseDelete();
		void		checkFileComplete();
		void		setStringToSent();
		void			doChunked();
		void			noChunked();
		void			setContentLength();
		void			encodeMessageBody();
		void			setHeader();
		void				setStringHeader();
		void				setStringStatusLine();


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

		std::string			_absolute_target;
		int					_file_oflag;
		AFdInfo::EventTypes	_file_event;

		RequestParser::header_field_t  _header_fields;
		std::string 		_http_version;
		int					_status_code;

		std::string			_string_to_send;
		std::string			_string_status_line;
		std::string 		_string_header;
		std::string			_message_body;

		bool				_header_sent;
		bool				_chunked;
};
