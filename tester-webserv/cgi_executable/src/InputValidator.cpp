#include "InputValidator.hpp"
#include "Output.hpp"
#include "status_codes.hpp"
#include "utility.hpp"
#include <cstdlib>
#include <string>
#include <unordered_set>

InputValidator::InputValidator()
: state(DEFAULT) {}

int InputValidator::getStatusCode() const {
	return status_code;
}

const std::string& InputValidator::getReason() const {
	return reason_phrase;
}

bool InputValidator::validInput(int argc, char *argv[]) {
	if (!validArguments(argc, argv)) {
		setError("invalid argument");
		return false;
	}
	if (!validEnvironment(argv[1])) {
		setError("invalid environment");
		return false;
	}
	return true;
}

bool InputValidator::validArguments(int argc, char *argv[]) {
	if (argc != 2) {
		return false;
	}
	return true;
}

bool InputValidator::validEnvironment(const std::string& arg) {
	if (!validRequestMethod()) {
		return false;
	} else if (!validPathInfo(arg)) {
		return false;
	} else if (!validContentLength()) {
		return false;
	}
	return true;
}

bool InputValidator::validRequestMethod() {
	const std::unordered_set<std::string> methods = {
		"GET",
		"HEAD",
		"POST",
		"PUT",
		"DELETE",
		"CONNECT",
		"OPTIONS",
		"TRACE",
		"PATCH"
	};

	char* method = getenv("REQUEST_METHOD");
	if (method == NULL) {
		setError("REQUEST_METHOD environment variable not found");
		return false;
	}

	if (methods.count(std::string(method)) == 0) {
		setError("Unknown method: " + std::string(method));
		return false;
	}
	return true;
}

bool InputValidator::validPathInfo(const std::string& arg) {
	char* pathinfo = getenv("PATH_INFO");
	if (pathinfo == NULL) {
		setError("PATH_INFO missing");
		return false;
	}
	char* real = realpath(("." + std::string(pathinfo)).c_str(), NULL);
	if (!real) {
		setError("real path of PATH_INFO not found: " + std::string(pathinfo));
		return false;
	}
	bool result = arg == real;
	if (!result) {
		setError("invalid PATH_INFO: " + std::string(real));
	}
	free(real);
	return result;
}

bool InputValidator::validContentLength() {
	char* length = getenv("CONTENT_LENGTH");
	if (!length) {
		return true;
	}

	for (size_t i = 0; length[i] != '\0'; ++i) {
		if (!isdigit(length[i])) {
			setError("invalid content length: " + std::string(length));
			return false;
		}
	}
	unsigned long value;
	if (util::strtoul(length, value) == -1) {
		setError("content length overflow: " + std::string(length));
		return false;
	}
	return true;
}

void InputValidator::setError(const std::string& reason) {
	if (state == InputValidator::ERROR) {
		return;
	}

	state = InputValidator::ERROR;
	status_code = StatusCode::BAD_REQUEST;
	reason_phrase = reason;
}
