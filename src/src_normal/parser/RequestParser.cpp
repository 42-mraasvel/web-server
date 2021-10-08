
#include "RequestParser.hpp"
#include "settings.hpp"

//TODO handle errors
/*
Return:
	- HEADER_INCOMPLETE if more data has to be read
	- HEADER_COMPLETE if the message-body is not complete
	- MESSAGE_COMPLETE if header_fields and message-body are both present
*/
int RequestParser::parseHeader(std::string const & request)
{
	if (request.find(EOHEADER) == std::string::npos)
	{
		return INCOMPLETE;
	}
	_method = GET;
	_target_resource = "/";
	_http_version = "HTTP/1.1";

	// _header_fields["content-length"] = "12";
	// _message_body = "Hello, World!";
	return OK;
}

enum RequestParser::MethodType RequestParser::getMethod() const
{
	return _method;
}

const std::string& RequestParser::getTargetResource() const
{
	return _target_resource;
}

const std::string& RequestParser::getHttpVersion() const
{
	return _http_version;
}

RequestParser::header_field_t& RequestParser::getHeaderFields()
{
	return _header_fields;
}

const std::string& RequestParser::getMessageBody() const
{
	return _message_body;
}
