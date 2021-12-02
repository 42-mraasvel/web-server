#pragma once
#include <string>
#include "webserver/MethodType.hpp"
#include "ConfigInfo.hpp"
#include "FileHandler.hpp"
#include "CgiHandler.hpp"

struct Request;

struct ResponseInfo
{
	ResponseInfo(Request const & request);
	
	enum Status
	{
		START,
		COMPLETE
	};

	enum Encoding
	{
		UNDEFINED,
		NOT_CHUNKED,
		CHUNKED
	};

	public:
		void	setStatus(Status new_status, int new_status_code);

	/* request related info */
	Method::Type		method;
	std::string			request_target;
	ConfigInfo			config_info;
	std::string 		http_version;
	bool				close_connection;

	/* other info */
	Status				status;
	int					status_code;
	std::string			effective_request_uri;
	bool				is_cgi;
	bool				error_page_attempted;
	Encoding			encoding;

	/* handler */
	iHandler*		handler;
	FileHandler		file_handler;
	CgiHandler		cgi_handler;

};
