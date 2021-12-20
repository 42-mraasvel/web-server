#pragma once

#include <string>

class Executor {
private:
	typedef void (Executor::*CommandFunction)(const std::string& data) const;

public:
	void execute(const std::string& body) const;

private:
	std::string extractCommand(const std::string& body) const;

	void executeMessageBody(const std::string& data) const;
	void executeStatusCode(const std::string& data) const;
	void executeTimeout(const std::string& data) const;
	void executeBadSyntax(const std::string& data) const;
	void executeCrash(const std::string& data) const;

private:
	static const std::string DELIMITER;

private:
	std::string command;
};
