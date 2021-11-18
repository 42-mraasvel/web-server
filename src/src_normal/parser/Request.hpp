#pragma once
# include "HeaderField.hpp"
# include "handler/ConfigResolver.hpp"

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
		typedef ConfigResolver::Address Address; //TODO_config: to incorporate with config class
	public:

		enum RequestStatus
		{
			READING,
			HEADER_COMPLETE,
			CONTINUE,
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

		static MethodType getMethodType(std::string const & s);

	/*
	TODO: No _prefix because public?
	*/

	public:
		Address			address;
		RequestStatus	status;
		int				status_code;
		// TODO: discuss whether this should remain a bool or be a flag
		bool			close_connection;

		MethodType		method;
		std::string		request_target;
		std::string		query;
		int				major_version;
		int				minor_version;
		header_field_t	header_fields;
		std::string		message_body;
};
