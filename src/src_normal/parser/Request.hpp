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
		std::string getStatusString() const;
		void print() const;
		void printBodyBytes() const;


	public:
		Request();

	/*
	TODO: No _prefix because public?
	*/

	public:
		RequestStatus	status;
		int				status_code;
		bool			processed;

		MethodType		method;
		std::string		target_resource;
		int				major_version;
		int				minor_version;
		header_field_t	header_fields;
		std::string		message_body;
};
