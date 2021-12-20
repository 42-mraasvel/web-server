#include "Response.hpp"
#include "color.hpp"
#include "Utility/Output.hpp"
#include "Utility/status_codes.hpp"

Response::Response()
:
major_version(0),
minor_version(0),
status_code(0),
reason_phrase("Default") {}

void Response::output(Output::Stream& out) const {
	out << "-- Response -- " << std::endl;
	out << "  ResponseLine: " << \
		"HTTP/" << major_version << "." << minor_version << \
		" " << status_code << " " << reason_phrase << std::endl;
	header_fields.output(out);
	out << "-- Message Body --" << std::endl;
	out << "  Body Size: " << message_body.size() << std::endl;
	if (message_body.size() > 10000) {
		out << "  Body too large to print" << std::endl;
	} else if (message_body.size() > 0) {
		out << "===" << std::endl << message_body << std::endl << "===" << std::endl;;
	}
	out << std::endl;
}

void Response::print() const {
	output(PRINT_DEBUG);
}

void Response::log() const {
	output(LOG);
}
