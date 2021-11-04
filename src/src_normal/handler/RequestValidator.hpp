#pragma once

struct Request;

class RequestValidator
{
    public:
		bool	isRequestValid(Request const & request);
    private:
		bool		isBadRequest(Request::RequestStatus status, int request_code);
		bool		isConnectionValid(Request const & request);
		bool		isHttpVersionValid(int http_major_version);
		bool		isMethodValid();
		bool			findMethod(MethodType method) const;
		bool		isExpectationValid(Request const & request);

    public:
        int getStatusCode() const;

    private:
        int                         _status_code;
		std::vector<std::string>	_allowed_methods; //TODO: to incorporate from Config
};
