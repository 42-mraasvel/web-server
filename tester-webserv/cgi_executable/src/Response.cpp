#include "Response.hpp"
#include "Output.hpp"
#include "status_codes.hpp"
#include <cassert>

void Response::write() const {
	write(this->body);
}

void Response::write(const std::string& message_body) const {
	assert(fields.size() > 0);
	for (auto it = fields.begin(); it != fields.end(); ++it) {
		PRINT << it->first << ": " << it->second << "\r\n";
	}
	PRINT << "\r\n";
	if (message_body.size() > 0) {
		PRINT << message_body;
	}
}

void Response::writeResponse(int status, const std::string& reason) {
	Response response;
	response.fields["Status"] = std::to_string(status);
	if (reason.size() > 0) {
		response.fields["Content-Length"] = std::to_string(reason.size());
	}
	response.write(reason);
}
