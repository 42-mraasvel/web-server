#pragma once
# include "HeaderField.hpp"
# include "handler/ConfigResolver.hpp"
# include "handler/ConfigInfo.hpp"

struct Request 
{
	public:
		typedef HeaderField header_field_t;
		typedef ConfigResolver::AddressType Address;
	public:

		enum RequestStatus
		{
			READING,
			EXPECT,
			TIME_OUT_REQUEST,
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
		Request(Address client, Address interface);

		static Method::Type getMethodType(std::string const & s);
	
	private:
		void init();

	public:
		Address			address;
		Address			interface_addr;
		RequestStatus	status;
		int				status_code;
		bool			close_connection;

		Method::Type	method;
		std::string		request_target;
		std::string		query;
		int				major_version;
		int				minor_version;
		header_field_t	header_fields;
		std::string		message_body;

		ConfigInfo		config_info;
};
