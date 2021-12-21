#pragma once
#include "request/Request.hpp"
#include <vector>

class ConfigResolver;

class RequestValidator
{
	public:
		RequestValidator();
    public:
		bool	isRequestValidPreConfig(Request const & request);
		bool	shouldCloseConnection() const;
		bool	shouldSendContinue() const;
    private:
		bool		isBadRequest(Request::RequestStatus status, int request_code);
		bool		isHostValid(Request const & request);
		bool		isConnectionValid(Request const & request);
		bool		isHttpVersionValid(int http_major_version);
		bool		isMethodValid(Method::Type const method);
		bool		isTransferEncodingValid(const HeaderField & header);
		bool		isContentCodingValid(const HeaderField & header);
		bool		isExpectationValid(Request const & request);

    public:
		bool	isRequestValidPostConfig(Request const & request);
    private:
		bool		isMethodAllowed(Request const & request);
		bool			findMethodInConfig(Method::Type const method, std::vector<std::string> const & allowed_methods) const;
    public:
        int getStatusCode() const;

    private:
        int		_status_code;
		bool	_close_connection;
		bool	_continue;
};
