#include "Executor.hpp"
#include "Response.hpp"
#include "Output.hpp"
#include "status_codes.hpp"
#include <stdexcept>
#include <unordered_map>

const std::string Executor::DELIMITER = "\r\n";

void Executor::execute(const std::string& body) const {
	static const std::unordered_map<std::string, CommandFunction> dispatch {
		{std::string("MessageBody"), &Executor::executeMessageBody},
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
