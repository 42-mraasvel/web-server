#pragma once
#include <string>
#include "parser/RequestParser.hpp"
#include "parser/Request.hpp"
#include "fd/AFdInfo.hpp"
#include "FileHandler.hpp"
#include "MediaType.hpp"
#include "RequestValidator.hpp"
#include "cgi/CgiHandler.hpp"

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

	public:
		Response(Request const & request);
	private:
		void	setHttpVersion(int minor_version);

	/* Client::readEvent() */
	public:
		void	initiate(Request const & request);
	private:
		void		resolveConfig(Request const & request);
		std::string const &	setAuthority(Request const & request, std::string const & default_server);
		void				setEffectiveRequestURI(std::string const & authority);
		void				setAbsoluteFilePath(std::string const & root, std::string const & default_file);
		void		evaluateConnectionFlag(Request const & request);
		int			validateRequest(Request const & request);
		void		processImmdiateResponse(Request const & request);
		bool			isRedirectResponse() const;
		void			processRedirectResponse();
		bool			isContinueResponse(Request const & request) const;
		void			processContinueResponse();

	public:
		void	executeRequest(FdTable & fd_table, Request & request);

	/* Client::writeEvent() */
	public:
		void	defineEncoding();

	public:
		void	generateResponse();
	private:
		void		evaluateExecutionError();
		void		setMessageBody();
		void			setHandlerMessageBody();
		void			setErrorPage();
		void		evaluateExecutionCompletion();
		void		setStringToSend();
		void			noChunked();
		void			doChunked();
		void			encodeMessageBody();
		void			setHeaderPart();
		void				setStatusCode();
		void				setStringStatusLine();
		void				setHeaderField();
		void					setDate();
		void					setConnection();
		void					setLocation();
		void					setRetryAfter();
		void					setAllow();
		void					setTransferEncodingOrContentLength();
		void						setContentLength();
		void					setContentType();
		void				setStringHeaderField();


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
		typedef	std::vector<std::string>::iterator				method_iterator;
		void				markComplete(int code);

	private:
		/* config related */
		MediaType::Map				_media_type_map;
		std::string					_authority;
		std::string					_effective_request_uri;
		std::string					_absolute_file_path;
		std::vector<std::string>	_allowed_methods; //TODO: to incorporate from Config

		/* info */
		MethodType			_method;
		std::string			_target_resource;
		RequestParser::header_field_t  _header_fields;
		std::string 		_http_version;
		int					_status_code;

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
