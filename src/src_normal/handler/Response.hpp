#pragma once
#include <string>
#include "fd/AFdInfo.hpp"
#include "MediaType.hpp"
#include "RequestValidator.hpp"
#include "ConfigResolver.hpp"
#include "FileHandler.hpp"
#include "cgi/CgiHandler.hpp"

class Response
{
	public:
		enum Status
		{
			START,
			COMPLETE
		};

	public:
		enum Chunk
		{
			UNDEFINED,
			NOT_CHUNKED,
			CHUNKED
		};

	public:
		Response(Request const & request);
	private:
		void	setHttpVersion(int minor_version);

	/* Client::readEvent() */
	public:
		void	executeRequest(FdTable & fd_table, Request & request);
	private:
		void		processCompleteRequest(FdTable & fd_table, Request & request);
		int				processCgiRequest(Request const & request);
		int				checkRequestTarget(Request const & request);
		void			setEffectiveRequestURI(Request const & request, std::string const & resolved_target);
		void			setAbsoluteFilePath(std::string const & resolved_file_path);

	/* Client::writeEvent() */
	public:
		void	generateResponse();
	private:
		void		noChunked();
		void		doChunked();
		void		encodeMessageBody();
		void		setHeaderPart();
		void			setStatusCode();
		void			setStringStatusLine();
		void			setHeaderField();
		void				setDate();
		void				setConnection();
		void				setLocation();
		void				setRetryAfter();
		void				setAllow();
		void				setTransferEncodingOrContentLength();
		void					setContentLength();
		void				setContentType();
		void			setStringHeaderField();


	/* Client::update() */
	public:
		void	update(FdTable & fd_table);
	private:
		void		setMessageBody(FdTable & fd_table);
		void			processRedirectResponse();
		void			processAutoIndex();
		bool			isErrorPageRedirected(FdTable & fd_table);
		void			setOtherErrorPage();
		void		setEncoding();
		bool			isReadyToBeChunked() const;

	/* utility */
	public:
		typedef HeaderField::const_iterator	header_iterator;
		bool					getCloseConnectionFlag() const;
		int						getStatusCode() const;
		std::string const &		getString() const;
		Method::Type const &	getMethod() const;
		void					clearString();
		bool					isComplete() const;
		bool					isReadyToWrite() const;
	private:
		typedef	std::vector<std::string>::const_iterator		method_iterator;
		void				markComplete(int code);

	private:
		/* config related */
		MediaType::Map				_media_type_map;
		std::string					_effective_request_uri;

		/* info */
		Method::Type		_method;
		std::string			_request_target;
		HeaderField			_header_fields;
		std::string 		_http_version;
		int					_status_code;
		ConfigInfo			_config_info;

		/* flags */
		Status				_status;
		bool				_header_part_set;
		Chunk				_encoding;
		bool				_close_connection;

		/* strings to send out */
		std::string			_string_to_send;
		std::string			_string_status_line;
		std::string 		_string_header_field;
		std::string			_message_body;

		/* handler */
		bool				_is_cgi;
		RequestValidator	_request_validator;
		FileHandler			_file_handler;
		CgiHandler			_cgi_handler;
		iHandler*			_handler;


};
