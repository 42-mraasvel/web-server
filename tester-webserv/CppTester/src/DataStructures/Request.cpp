#include "Request.hpp"
#include "color.hpp"
#include "Utility/Output.hpp"

Request::Settings::Settings()
: flags(NONE) {}

Request::Request(Address server_addr)
: server_addr(server_addr) {}

void Request::log() const {
	output(LOG);
}

void Request::print() const {
	output(PRINT_DEBUG);
}

void Request::output(Output::Stream& out) const {
	out << "-- Request --" << std::endl;
	out << tag << " : " << name << std::endl;
	out << "Authority: " << server_addr.ip << ":" << server_addr.port << std::endl;
	out << "RequestLine: " << request_line << std::endl;
	header_fields.output(out);
	out << "-- Message Body --" << std::endl;
	out << "Body Size: " << message_body.size() << std::endl;
	if (message_body.size() > 10000) {
		out << "Body too large to print" << std::endl;
	} else if (message_body.size() > 0) {
		out << ">>>" << std::endl << message_body << std::endl << "<<<" << std::endl;
	}
	out << std::endl;
}
