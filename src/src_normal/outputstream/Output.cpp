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
#ifdef PRINT_INFO_FLAG
			// std::cout << YELLOW_BOLD "[INFO]: " RESET_COLOR << os.str();
			std::cout << os.str();
#endif /* PRINT_INFO_FLAG */
			break;
		case Output::DEBUG:
#ifdef PRINT_DEBUG_FLAG
			std::cout << BLUE_BOLD "[DEBUG]: " RESET_COLOR << os.str();
#endif
			break;
	}
}

std::ostringstream& Output::get(Level l)
{
	level = l;
	return os;
}
