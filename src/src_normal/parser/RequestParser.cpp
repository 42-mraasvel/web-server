
#include "RequestParser.hpp"
#include "settings.hpp"

//TODO Build parser that really works
//TODO handle errors
int RequestParser::parseHeader(std::string const & request)
{
	_http_version = "HTTP/1.1";

	int	lemon = 4;

	switch (lemon)
	{
		case 1:
			_method = GET;
			_target_resource = "./page_sample/Anneaa";
			break;
		case 2:
			_method = GET;
			_target_resource = "./page_sample/funny";
			break;
		case 3:
			_method = POST;
			_target_resource = "./page_sample/moonlight";
			_message_body = "Pyxis is awesome!\n";
			break;
		case 4:
			_method = DELETE;
			_target_resource = "./page_sample/moonlight";
			break;
	}	

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
