#pragma once
#include "parser/Request.hpp"
#include <vector>
#include <string>

class RequestValidator
{
    public:
		bool	isRequestValidPreConfig(Request const & request);
    private:
		bool		isBadRequest(Request::RequestStatus status, int request_code);
		bool		isHostValid(Request const & request);
		bool		isConnectionValid(Request const & request);
		bool		isHttpVersionValid(int http_major_version);
		bool		isMethodValid(MethodType const method);
		bool		isExpectationValid(Request const & request);
    public:
		bool	isRequestValidPostConfig(Request const & request);
		bool		isMethodAllowed(MethodType const method);
		bool			findMethod(MethodType const method) const;

    public:
        int getStatusCode() const;

    private:
        int                         _status_code;
		std::vector<std::string>	_allowed_methods; //TODO: to incorporate from Config
};
