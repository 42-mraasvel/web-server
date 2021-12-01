#pragma once
#include <string>
#include "parser/HeaderField.hpp"
#include "ResponseInfo.hpp"
#include "ConfigInfo.hpp"

class iHandler;

class ResponseGenerator
{
	public:
		typedef HeaderField::const_iterator		header_iterator;
		typedef	std::vector< std::string>		method_type;

	public:
		ResponseGenerator();

	/* Client::writeEvent() */
	public:
		void	generateResponse(ResponseInfo const & info, iHandler* handler);
	private:
		void		noChunked(ResponseInfo const & info, iHandler* handler);
		void		doChunked(ResponseInfo const & info, iHandler* handler);
		void		encodeMessageBody(ResponseInfo::Status status);
		void		setHeaderPart(ResponseInfo const & info, iHandler* handler);
		void			setStatusCode(ResponseInfo const & info, iHandler* handler);
		void			setStringStatusLine(std::string const & http_version);
		void			setHeaderField(ResponseInfo const & info, iHandler* handler);
		void				setDate();
		void				setConnection(bool close_connection);
		void				setLocation(ResponseInfo const & info);
		void				setRetryAfter();
		void				setAllow(ConfigInfo const & config_info);
		void				setTransferEncodingOrContentLength(ResponseInfo::Encoding encoding);
		void					setContentLength();
		void				setContentType(ConfigInfo::ConfigResult result);
		void			setStringHeaderField();
	public:
		void	appendString(std::string & append_to);

	public:
		std::string			message_body;
	private:
		int					_status_code;
		HeaderField			_header_fields;
		bool				_header_part_set;
		std::string			_string_to_send;
		std::string			_string_status_line;
		std::string 		_string_header_field;
};
