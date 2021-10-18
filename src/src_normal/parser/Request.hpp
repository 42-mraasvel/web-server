#pragma once

# include <map>
# include <string>
# include <functional>

struct case_insensitive_less
	: public std::binary_function<std::string, std::string, bool>
{
	bool operator() (const std::string& x, const std::string& y) const;
};

enum MethodType
{
	GET,
	POST,
	DELETE,
	OTHER
};

struct Request 
{
public:
	typedef std::map<
			std::string, std::string,
			case_insensitive_less> header_field_t;

public:
	enum RequestStatus
	{
		READING,
		HEADER_COMPLETE,
		COMPLETE,
		BAD_REQUEST
	};

public:
	Request();

/*
TODO: No _prefix because public?
*/

public:
	RequestStatus	status;

	MethodType		method;
	std::string		target_resource;
	int				major_version;
	int				minor_version;
	header_field_t	header_fields;
	std::string		message_body;
};
