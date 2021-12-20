#pragma once

#include <string>

class InputValidator {
private:
	enum State {
		DEFAULT,
		ERROR
	};

public:
	InputValidator();
	bool validInput(int argc, char *argv[]);

	int getStatusCode() const;
	const std::string& getReason() const;

private:
	bool validArguments(int argc, char *argv[]);
	bool validEnvironment(const std::string& arg);
	bool validRequestMethod();
	bool validPathInfo(const std::string& arg);
	bool validContentLength();

	void setError(const std::string& reason);

private:
	State state;
	int status_code;
	std::string reason_phrase;
};
