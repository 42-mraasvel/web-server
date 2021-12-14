#include "Executor.hpp"
#include "InputValidator.hpp"
#include "Response.hpp"
#include "Output.hpp"
#include "settings.hpp"
#include "status_codes.hpp"
#include <iostream>
#include <cassert>
#include <unistd.h>
#include <cstdio>

static int readContent(std::string* body) {
	assert(body);

	std::size_t len = std::stoull(getenv("CONTENT_LENGTH"));
	std::size_t bytes_read = 0;
	while (bytes_read != len) {
		std::string buffer(BUFFER_SIZE, '\0');
		ssize_t n = read(STDIN_FILENO, &buffer[0], BUFFER_SIZE);
		if (n == -1) {
			perror("read");
			return -1;
		} else if (n == 0) {
			Response::writeResponse(StatusCode::INTERNAL_SERVER_ERROR,
				"didn't receive CONTENT_LENGTH bytes");
			return -1;
		} else if (bytes_read + n > len) {
			Response::writeResponse(StatusCode::INTERNAL_SERVER_ERROR,
				"too many bytes read: " + std::to_string(bytes_read + n) + ": expected: " + std::to_string(len));
			return -1;
		}
		body->append(buffer, 0, n);
		bytes_read += n;
	}
	return 0;
}

int main(int argc, char *argv[], char *environ[]) {
	InputValidator validator;
	if (!validator.validInput(argc, argv)) {
		Response::writeResponse(validator.getStatusCode(), validator.getReason());
		return 0;
	}

	std::string body;
	if (getenv("CONTENT_LENGTH")) {
		readContent(&body);
	}

	try {
		Executor executor;
		executor.execute(body);
	} catch (const std::exception& e) {
		Response::writeResponse(StatusCode::INTERNAL_SERVER_ERROR, e.what());
	}
	return 0;
}
