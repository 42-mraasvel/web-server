#include "Output.hpp"
#include "settings.hpp"
#include <iostream>

Output::Output() {}
Output::~Output() {
	switch (level)
	{
		case Output::DEFAULT:
			std::cout << os.str();
			break;
		case Output::ERROR:
			std::cerr << RED_BOLD "[ERROR]: " RESET_COLOR <<  os.str();
			break;
		case Output::WARNING:
			std::cerr << RED_BOLD "[WARNING]: " RESET_COLOR <<  os.str();
			break;
		case Output::INFO:
			std::cout << YELLOW_BOLD "[INFO]: " RESET_COLOR << os.str();
			break;
		case Output::DEBUG:
			// std::cout << BLUE_BOLD "[DEBUG]: " RESET_COLOR << os.str();
			break;
	}
}

std::ostringstream& Output::get(Level l)
{
	level = l;
	return os;
}
