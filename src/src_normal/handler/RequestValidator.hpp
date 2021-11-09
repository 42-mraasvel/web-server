#pragma once
#include "parser/Request.hpp"
#include <vector>
#include <string>

class RequestValidator
{
    public:
		bool	isRequestValid(Request const & request);
    private:
		bool		isBadRequest(Request::RequestStatus status, int request_code);
		bool		isHostValid(Request const & request);
		bool		isConnectionValid(Request const & request);
		bool		isHttpVersionValid(int http_major_version);
		bool		isMethodValid(MethodType const method);
		bool			findMethod(MethodType const method) const;
		bool		isExpectationValid(Request const & request);

    public:
        int getStatusCode() const;

    private:
        int                         _status_code;
		std::vector<std::string>	_allowed_methods; //TODO: to incorporate from Config
};
