#include "Output.hpp"
#include "color.hpp"
#include <iostream>
#include <fstream>
#include <cassert>

Output::Output(Type type)
: type(type) {}

Output::~Output() {
	if (type == Type::LOGGER) {
		log();
	} else {
		console();
	}
}

void Output::clearLog() {
	std::fstream fs(LOGFILE, std::ios_base::trunc | std::ios_base::out);
	fs.close();
}

void Output::log() const {
	std::ofstream ofs(LOGFILE, std::ios_base::app | std::ios_base::out);
	assert(ofs.is_open());
	if (level == DEFAULT) {
		ofs << os.str();
	} else {
		ofs << getLevelString() << " " << os.str();
	}
	ofs.close();
}

void Output::console() const {
	switch (level) {
		case Output::ERROR:
		case Output::WARNING:
			print(RED_BOLD, std::cerr);
			return;
		case Output::INFO:
			print(YELLOW_BOLD, std::cout);
			break;
		case Output::DEBUG:
			print(BLUE_BOLD, std::cout);
			break;
		default:
			print("", std::cout);
			break;
	}
}

void Output::print(const std::string& color, std::ostream& out) const {
	if (level == DEFAULT) {
		out << os.str();
	} else {
		out << color << getLevelString() << ": " << RESET_COLOR << os.str();
	}
}

std::string Output::getLevelString() const {
	switch (level) {
		case Output::DEFAULT:
			return "";
		case Output::ERROR:
			return "[ERROR]";
		case Output::WARNING:
			return "[WARNING]";
		case Output::INFO:
			return "[INFO]";
		case Output::DEBUG:
			return "[DEBUG]";
	}
}

std::ostringstream& Output::get(Level level)
{
	this->level = level;
	return os;
}
