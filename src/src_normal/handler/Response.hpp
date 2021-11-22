#pragma once
#include <string>
#include "parser/RequestParser.hpp"
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
			HEADER_COMPLETE,
			COMPLETE
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
		void			setEffectiveRequestURI(std::string const & host, int port, std::string const & resolved_target);
		void			setAbsoluteFilePath(std::string const & root, std::string const & resolved_file_path);
		void			handlerExecution(FdTable & fd_table, Request & request);
		bool			isTargetExist(Request & request);

	/* Client::writeEvent() */
	public:
		void	defineEncoding();

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
		void		evaluateExecutionError();
		void		setMessageBody(FdTable & fd_table);
		void			processRedirectResponse();
		void			processAutoIndex();
		bool			isErrorPageRedirected(FdTable & fd_table);
		void			setOtherErrorPage();
		void			setHandlerMessageBody();
		void		evaluateExecutionCompletion();

	/* utility */
	public:
		typedef RequestParser::header_field_t::const_iterator	header_iterator;
		bool				getCloseConnectionFlag() const;
		int					getStatusCode() const;
		std::string const &	getString() const;
		void				clearString();
		bool				isComplete() const;
		bool				isHandlerReadyToWrite() const;
	private:
		typedef	std::vector<std::string>::const_iterator		method_iterator;
		void				markComplete(int code);

	private:
		/* config related */
		MediaType::Map				_media_type_map;
		std::string					_effective_request_uri;

		/* info */
		MethodType			_method;
		std::string			_request_target;
		RequestParser::header_field_t  _header_fields;
		std::string 		_http_version;
		int					_status_code;
		ConfigInfo			_config_info;

		/* flags */
		Status				_status;
		bool				_header_part_set;
		bool				_chunked;
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


};
