#include "client.hpp"
#include <cstdarg>
#include <unistd.h>
#include <iostream>
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
	return OK;
}
