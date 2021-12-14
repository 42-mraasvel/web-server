#pragma once

#include <string>

class Executor {
private:
	typedef void (Executor::*CommandFunction)(const std::string& data) const;

public:
	void execute(const std::string& body) const;

private:
	void executeMessageBody(const std::string& data) const;
	std::string extractCommand(const std::string& body) const;

private:
	static const std::string DELIMITER;

private:
	std::string command;
};
