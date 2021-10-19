#pragma once
#include <string>
#include "parser/RequestParser.hpp"
#include "AFdInfo.hpp"

class File;

struct Response
{
	public:
	enum Status
	{
		NOT_COMPLETE,
		COMPLETE
	};

	public:
		File*				file;
		Status				status;

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
		Response();
};
