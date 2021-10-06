
#include "RequestParser.hpp"
#include "settings.hpp"



//TODO Build parser that really works
//TODO handle errors
int RequestParser::parseHeader(std::string const & request)
{
    _method = GET;
    _target_resource = "/";
    _http_version = "HTTP/1.1";
    return OK;
}

enum RequestParser::MethodType RequestParser::getMethod() const
{
    return _method;
}











