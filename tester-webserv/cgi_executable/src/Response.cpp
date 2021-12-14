#include "Response.hpp"
#include "Output.hpp"
#include <cassert>

void Response::write() const {
	assert(fields.size() > 0);
	for (auto it = fields.begin(); it != fields.end(); ++it) {
		PRINT << it->first << ": " << it->second << "\r\n";
	}
	PRINT << "\r\n";
	PRINT << body;
}

void Response::errorResponse(int status, const std::string& reason) {
	Response response;

	response.fields["Status"] = std::to_string(status);
	response.body = reason + "\r\n";
	response.fields["Content-Length"] = std::to_string(response.body.size());
	response.write();
}