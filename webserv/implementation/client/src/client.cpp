#include "client.hpp"
#include "UserSettings.hpp"
#include <cstdio>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <chrono>

Client::Client(Settings* settings)
: settings(settings), file_index(0), fd(-1), eof_reached(false), nothing_read(false) {
	assert(settings != nullptr);
}

Client::~Client() {}

void Client::setFiles(int argc, const char* const* argv) {
	for (int i = 0; i < argc; ++i) {
		files.push_back(argv[i]);
	}
}

/*
1. Read in data while available (no EOF on current FD), or if we can send a full BUFFER_SIZE
2. Send what's currently in the buffer (BUFFER_SIZE), erase it from the buffer
*/

static int elapsedTime(std::chrono::steady_clock::time_point begin) {
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	return std::chrono::duration_cast<std::chrono::seconds>(end - begin).count();
}

void Client::timeoutSingleConnection() {
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	while (elapsedTime(begin) < settings->getReadTimeout()) {
		readResponse();
		if (nothing_read) {
			printf(RED_BOLD "Timeout to exit..." RESET_COLOR " %d\n", settings->getReadTimeout() - elapsedTime(begin));
			sleep(1);
		} else {
			// reset timeout if something was read
			begin = std::chrono::steady_clock::now();
		}
	}
}

int Client::run() {
	if (openNextConnection() == ERR) {
		return ERR;
	}
	if (!settings->getStdin() && files.empty()) {
		return putError("ERROR: no request files\n");
	}

	while (!finished()) {
		if (readRequest() == ERR) {
			break;
		}
		if (sendRequest() == ERR) {
			break;
		}
		//TODO: add sleep time as parameter
		// sleep(1);
		// If there is anything to read, we should read it and discard it so to keep the buffer empty
		if (readResponse() == ERR) {
			break;
		}
	}

	// Give the server time to respond
	if (settings->getSingleConnection()) {
		timeoutSingleConnection();
	}

	connection.closeConnection();
	if (!finished()) {
		return ERR;
	}
	return OK;
}

/* Request Reading */

int Client::readRequest() {
	if (settings->getStdin()) {
		return readStdin();
	} else {
		return readFile();
	}
	return OK;
}

int Client::readStdin() {
	while (request.size() < settings->getBufferSize()) {
		ssize_t n = read(STDIN_FILENO, buffer, BUFFER_SIZE);
		if (n == -1) {
			perror("stdin: read");
			return ERR;
		}
		buffer[n] = '\0';
		if (n == 0) {
			eof_reached = true;
			break;
		}
		request.append(buffer);
		replaceNewlines(request);
	}
	return OK;
}

int Client::readFile() {
	if (!fileOpen()) {
		// If connection == MULTIPLE, open next connection
		if (openNextFile() == ERR) {
			return ERR;
		}
		if (file_index != 0 && !settings->getSingleConnection()) {
			if (openNextConnection() == ERR) {
				return ERR;
			}
		}
	}

	while (request.size() < settings->getBufferSize()) {
		ssize_t n = read(fd, buffer, BUFFER_SIZE);
		if (n == -1) {
			perror(("read: " + files[file_index - 1]).c_str());
			return ERR;
		}
		buffer[n] = '\0';
		if (n == 0) {
			closeFile();
			break;
		}
		request.append(buffer);
		replaceNewlines(request);
	}
	return OK;
}

int Client::openNextFile() {
	if (PRINT) {
		printf(YELLOW_BOLD "Opening: %s" RESET_COLOR "\n", files[file_index].c_str());
	}
	fd = open(files[file_index].c_str(), O_RDONLY);
	if (fd == -1) {
		perror(("open: " + files[file_index]).c_str());
		return ERR;
	}
	file_index += 1;
	return OK;
}

void Client::closeFile() {
	if (close(fd) == -1) {
		perror("close");
	}
	fd = -1;
}

// Replace '\n' with '\r\n' for valid requests
void Client::replaceNewlines(std::string& str) {
	std::size_t index = 0;
	while (index < str.size()) {
		index = str.find("\n", index);
		if (index == std::string::npos) {
			break;
		}

		if ((index != 0 && str[index - 1] == '\r')) {
			index += 1;
			continue;
		}
		str.replace(index, 1, "\r\n");
		index += 2;
	}
}

int Client::sendRequest() {
	if (PRINT_REQUEST) {
		std::string sub = request.substr(0, settings->getBufferSize());
		printf(GREEN_BOLD "SENDING" RESET_COLOR"\n");
		printMessage(sub);
	}
	ssize_t n = connection.sendRequest(request, std::min(settings->getBufferSize(), request.size()));
	if (n == ERR) {
		return ERR;
	}
	request.erase(0, n);
	return OK;
}

int Client::openNextConnection() {
	if (PRINT) {
		printf(GREEN_BOLD "Opening a new connection..." RESET_COLOR "\n");
	}
	if (connection.initConnection(settings->getHost(), settings->getPort(), !settings->getSingleConnection()) == ERR) {
		return ERR;
	}
	return OK;
}

int Client::readResponse() {

	std::string response;
	ssize_t n = connection.receiveResponse(response);
	if (n == ERR) {
		return ERR;
	} else if (n != 0) {
		nothing_read = false;
		if (PRINT_RESPONSE) {
			printf(GREEN_BOLD "RESPONSE" RESET_COLOR "\n");
			printMessage(response);
		}
	} else {
		nothing_read = true;
	}
	return OK;
}

void Client::printMessage(const std::string& x) const {
	if (!PRINT_UNPRINTABLE) {
		printf("[");
	}
	for (std::size_t i = 0; i < x.size(); ++i) {
		if (!isprint(x[i]) && !PRINT_UNPRINTABLE) {
			printf("[%d]", x[i]);
		} else {
			printf("%c", x[i]);
		}
	}
	if (!PRINT_UNPRINTABLE) {
		printf("]\n");
	} else {
		printf("\n");
	}
}

/*
Cases for being finished:
	- EOF encountered in stdin
	- No files left to read
*/

bool Client::finished() const {
	if (request.size() != 0) {
		return false;
	}
	if (settings->getStdin()) {
		return eof();
	} else {
		return file_index == files.size();
	}
}

bool Client::eof() const {
	return eof_reached;
}

bool Client::fileOpen() const {
	return fd != -1;
}
