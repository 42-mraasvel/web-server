#pragma once
# include "HeaderField.hpp"

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
		typedef HeaderField header_field_t;
		typedef std::pair< std::string, int > Address; //TODO_config: to incorporate with config class
	public:

		enum RequestStatus
		{
			READING,
			HEADER_COMPLETE,
			COMPLETE,
			BAD_REQUEST
		};

	/* DEBUGGING */
	public:
		std::string getMethodString() const;
		std::string getProtocolString() const;
		std::string getStatusString() const;
		void print() const;
		void printBodyBytes() const;


	public:
		Request();

	/*
	TODO: No _prefix because public?
	*/

	public:
		Address			address;
		RequestStatus	status;
		int				status_code;

		MethodType		method;
		std::string		request_target;
		std::string		query;
		int				major_version;
		int				minor_version;
		header_field_t	header_fields;
		std::string		message_body;
};
