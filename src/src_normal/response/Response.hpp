#pragma once
#include "handler/FileHandler.hpp"
#include "handler/CgiHandler.hpp"

struct Request;

struct Response
{
	Response(Request const & request);
	
	enum Status
	{
		START,
		COMPLETE,
		SPECIAL
	};

	enum Encoding
	{
		UNDEFINED,
		NOT_CHUNKED,
		CHUNKED
	};

	public:
		void	markComplete(int new_status_code);
		void	markSpecial(int new_status_code);
		bool	isFinished() const;
		void	setCgi();
		void	unsetCgi();
		void	resetErrorPageRedirection();
		void	print() const;

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
	bool				content_type_fixed;

	/* handler */
	iHandler*		handler;
	FileHandler		file_handler;
	CgiHandler		cgi_handler;

	/* content */
	HeaderField			header_fields;
	bool				header_part_set;
	std::string			string_to_send;
	std::string			string_status_line;
	std::string 		string_header_field;
	std::string			message_body;

};
