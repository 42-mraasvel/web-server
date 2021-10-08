#pragma once
#include <string>
#include <map>

class RequestParser
{
	public:
		enum MethodType
		{
			GET,
			POST,
			DELETE,
			OTHER
		};

		enum CompletionStatus
		{
			HEADER_INCOMPLETE,
			HEADER_COMPLETE,
			REQUEST_COMPLETE
		};

		typedef std::map<std::string, std::string> header_field_t;

	public:

		int					parseHeader(std::string const &request);
		enum MethodType		getMethod() const;
		const std::string&	getTargetResource() const;
		const std::string&	getHttpVersion() const;
		header_field_t&		getHeaderFields();
		const std::string&	getMessageBody() const;

	private:
		enum MethodType _method;
		std::string _target_resource;
		std::string _http_version;
		std::map<std::string, std::string>  _header_fields;
		std::string _message_body;
};
