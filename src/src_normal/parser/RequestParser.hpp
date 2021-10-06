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
    public:
        int parseHeader(std::string const &request);
        enum MethodType getMethod() const; 

    private:
        enum MethodType _method;
        std::string _target_resource;
        std::string _http_version;
        
        std::map<std::string, std::string>  _header_fields;
};
