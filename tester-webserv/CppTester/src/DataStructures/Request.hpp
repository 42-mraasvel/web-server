#pragma once

#include "HeaderField.hpp"
#include "Address.hpp"
#include <string>
#include <memory>

/*
Members:
	- server_addr: contains ip and port of the server
	- request_line: `[METHOD] SP [REQUEST_TARGET] SP [HTTP_VERSION]
	- header_fields: Map of all header fields
	- message_body: message body sent in the request (raw data)
*/
struct Request {
	public:
		typedef std::shared_ptr<Request> Pointer;

	public:
		Request(Address server_addr);

		void print() const;
		void log() const;
		void output(Output::Stream& out) const;

	public:
		Address server_addr;
		std::string request_line;
		HeaderField header_fields;
		std::string message_body;

		std::string name;
		std::string tag;
};
