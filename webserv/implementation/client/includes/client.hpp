#pragma once

#include "connection.hpp"
#include "settings.hpp"
#include <vector>

class Client {
public:
	Client(Settings* settings);
	~Client();

	void setFiles(int argc, const char* const* argv);

	int run();

private:

	bool finished() const;
	bool eof() const;
	bool fileOpen() const;

	int readRequest();
	int sendRequest();
	int readResponse();
	int openNextConnection();
	void timeoutSingleConnection();

	int readStdin();
	int readFile();
	int openNextFile();
	void closeFile();
	void replaceNewlines(std::string& str);
	void printMessage(const std::string& x) const;

private:
	Connection connection;
	Settings* settings;

	char buffer[BUFFER_SIZE + 1];
	std::string request;

	std::vector<std::string> files;
	std::size_t file_index;
	int fd;
	bool eof_reached;
	bool nothing_read;
};
