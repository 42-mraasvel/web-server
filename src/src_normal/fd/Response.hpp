#pragma once
#include <string>
#include "parser/RequestParser.hpp"
#include "AFdInfo.hpp"
#include "parser/Request.hpp"

class File;

struct Response
{
	public:
	enum Status
	{
		HEADER_PENDING,
		HEADER_COMPLETE,
		COMPLETE
	};

	public:
		File*				file;
		Status				status;
		MethodType			method;

		//TODO: add time last active for TIMEOUT
		std::string			absolute_target;
		int					file_oflag;
		AFdInfo::EventTypes	file_event;

		RequestParser::header_field_t  header_fields;

		std::string 		http_version;
		int					status_code;
		std::string 		header_string;
		std::string			message_body;

		std::string			string;

	public:
		Response(Request const & request);
		~Response();
	
		int	createFile();
	
	private:
		void	setHttpVersion(int minor_version);
		void	previewMethod();
		void	generateAbsoluteTarget(std::string const & target_resourse);
};
