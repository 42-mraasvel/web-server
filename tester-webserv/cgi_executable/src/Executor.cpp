#include "Executor.hpp"
#include "Response.hpp"
#include "Output.hpp"
#include "status_codes.hpp"
#include <unistd.h>
#include <stdexcept>
#include <unordered_map>

const std::string Executor::DELIMITER = "\r\n";

void Executor::execute(const std::string& body) const {
	static const std::unordered_map<std::string, CommandFunction> dispatch {
		{std::string("MessageBody"), &Executor::executeMessageBody},
		{std::string("StatusCode"), &Executor::executeStatusCode},
		{std::string("Timeout"), &Executor::executeTimeout},
		{std::string("BadSyntax"), &Executor::executeBadSyntax},
		{std::string("Crash"), &Executor::executeCrash},
	};

	const std::string command = extractCommand(body);
	if (dispatch.count(command) == 0) {
		throw std::runtime_error("unrecognized command: [" + command + "]");
	}
	(this->*(dispatch.at(command)))(body);
}

std::string Executor::extractCommand(const std::string& body) const {
	std::size_t index = body.find(DELIMITER);
	if (index == std::string::npos) {
		throw std::runtime_error("invalid body: " + body);
	}
	return body.substr(0, index);
}

/*
Commands
*/

void Executor::executeMessageBody(const std::string& data) const {
	std::size_t start = data.find(DELIMITER) + DELIMITER.size();
	Response::writeResponse(StatusCode::STATUS_OK, data.substr(start));
}

void Executor::executeStatusCode(const std::string& data) const {
	std::size_t start = data.find(DELIMITER) + DELIMITER.size();
	Response::writeResponse(std::stoi(data.substr(start)), "");
}

void Executor::executeTimeout(const std::string& data) const {
	std::size_t start = data.find(DELIMITER) + DELIMITER.size();
	std::size_t time = std::stoull(data.substr(start));
	sleep(time);
	Response::writeResponse(StatusCode::INTERNAL_SERVER_ERROR, "Waited: " + data.substr(start) + " seconds and didn't time out");
}

void Executor::executeBadSyntax(const std::string& data) const {
	std::size_t n = 4096;
	std::string s(n, 'a');
	for (std::size_t i = 0; i < 100; ++i) {
		PRINT << s;
	}
}

void Executor::executeCrash(const std::string& data) const {
	std::exit(1);
}
