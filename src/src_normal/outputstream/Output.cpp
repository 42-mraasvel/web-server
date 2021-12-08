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
			if (PRINT_INFO_FLAG)
			{
				std::cout << os.str();
			}
			break;
		case Output::DEBUG:
			if (PRINT_DEBUG_FLAG)
			{
				std::cout << os.str();
			}
			break;
	}
}

std::ostringstream& Output::get(Level l)
{
	level = l;
	return os;
}
