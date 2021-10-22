#include "client.hpp"
#include "settings.hpp"
#include "UserSettings.hpp"
#include <cstdarg>
#include <cstdio>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>

int putError(const char* format, ...) {
	va_list ap;
	va_start(ap, format);
	vdprintf(STDERR_FILENO, format, ap);
	va_end(ap);
	return ERR;
}

void printFirstLine(const std::string& x) {
	std::size_t index = x.find(CRLF);
	if (index == std::string::npos) {
		index = x.find('\n');
	}
	if (index == std::string::npos) {
		std::cout << x << std::endl;
	} else {
		std::cout << x.substr(0, index) << std::endl;
	}
	std::cout << std::endl;
}

void printHeader(std::ostream& out, const std::string& response) {
	std::size_t index = response.find(CRLF CRLF);
	out << response.substr(0, index) << std::endl << std::endl;
}

bool isFile(const std::string& pathname) {
	struct stat s;
	int result = stat(pathname.c_str(), &s);
	if (result == ERR) {
		perror(pathname.c_str());
		return false;
	}

	if (s.st_mode & S_IFDIR || !(s.st_mode & S_IFREG)) {
		putError("client: is a directory: %s\n", pathname.c_str());
		return false;
	}

	return true;
}

int toInt(const std::string& rhs) {
	std::stringstream ss;
	ss << rhs;

	int x;
	ss >> x;
	return x;
}

void replaceNewlines(std::string& str) {
	std::size_t index = 0;
	while (index < str.size()) {
		index = str.find("\n", index);
		if (index == std::string::npos) {
			break;
		}

		if ((index != 0 && str[index - 1] == '\r')) {
			continue;
		}
		str.replace(index, 1, "\r\n");
		index += 2;
	}
}

int readFile(const std::string& filename, std::string& request) {

	if (!isFile(filename)) {
		return ERR;
	}

	std::ifstream ifs(filename.c_str());
	if (!ifs.is_open()) {
		return putError("Not able to open file: %s\n", filename.c_str());
	}

	request = std::string(	std::istreambuf_iterator<char>(ifs),
							std::istreambuf_iterator<char>());

	replaceNewlines(request);
	if (request.find(EOHEADER) == std::string::npos) {
		return putError("client: %s: no [CRLF CRLF] (end of header-field) found\r\n", filename.c_str());
	}
	return OK;
}

// int sendRequest(const std::string& request, const std::string& host, int port) {
// 	Client client;
// 	int id = client.newConnection(host, port);
// 	if (id == ERR) {
// 		return ERR;
// 	}

// 	client.sendRequest(id, request);

// 	std::string response = client.receiveResponse(id);
// 	if (PRINT) {
// 		std::cout << YELLOW_BOLD "Response: (HEADER ONLY)" RESET_COLOR << std::endl;
// 		if (PRINT_MESSAGE_BODY) {
// 			std::cout << response << std::endl;
// 		} else {
// 			printHeader(std::cout, response);
// 		}
// 	}
// 	return OK;
// }

int main(int argc, char *argv[]) {
	if (argc < 2) {
		return putError("Usage: %s [OPTIONS] [REQUEST_FILE]+\r\n", argv[0]);
	}

	Settings settings;
	int i = settings.parseFlags(argc, argv);
	if (i == -1) {
		return ERR;
	}

	if (PRINT) {
		settings.print();
	}

	Client client(&settings);

	if (!settings.getStdin()) {
		client.setFiles(argc - i, argv + i);
	}

	if (client.run() == ERR) {
		return ERR;
	}

	// if (i >= argc) {
	// 	return putError("Error: No request files\r\n");
	// }

	// Client client;


	// for (; i < argc; ++i) {
	// 	if (PRINT) {
	// 		printf(YELLOW_BOLD "Request #%d: %s" RESET_COLOR "\r\n", i, argv[i]);
	// 	}
	// 	std::string request;
	// 	if (readFile(argv[i], request) == ERR) {
	// 		std::cout << std::endl;
	// 		continue;
	// 	}
	// 	if (PRINT_REQUEST) {
	// 		std::cout << request << std::endl;
	// 	}
	// 	sendRequest(request, sets.getHost(), sets.getPort());
	// }
	return OK;
}
