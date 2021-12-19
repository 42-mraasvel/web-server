#pragma once

#include <unordered_map>
#include <string>

struct Response {
	typedef std::unordered_map<std::string, std::string> header_type;

	void write() const;
	void write(const std::string& message_body) const;
	static void writeResponse(int status, const std::string& reason);

	header_type fields;
	std::string body;
};
