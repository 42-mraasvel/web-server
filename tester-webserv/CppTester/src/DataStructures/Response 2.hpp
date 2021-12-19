#pragma once

#include "HeaderField.hpp"
#include "Utility/Output.hpp"
#include <string>
#include <memory>

struct Response {

	public:
		typedef std::shared_ptr<Response> Pointer;

	public:
		Response();

		void print() const;
		void log() const;
		void output(Output::Stream& out) const;

	public:
		int major_version;
		int minor_version;
		int status_code;
		std::string reason_phrase;
		HeaderField header_fields;
		std::string message_body;
};
