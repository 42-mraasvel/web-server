#include "Output.hpp"
#include "settings.hpp"
#include "utility/color.hpp"
#include <iostream>
#include <fstream>

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
			std::cout << os.str();
#endif /* PRINT_INFO_FLAG */
			break;
		case Output::DEBUG:
#ifdef PRINT_DEBUG_FLAG
			std::cout << os.str();
#endif
			break;
	}
}

void Output::log() const
{
	std::ofstream ofs(LOGFILE, std::ios_base::app);
	if (!ofs.is_open())
	{
		PRINT << "Can't open file: " << LOGFILE << std::endl;
		return;
	}
	ofs << "[" << getLevelString() << "]: " << os.str();
	ofs.close();
}

std::string Output::getLevelString() const
{
	switch (level)
	{
		case DEFAULT:
			return "";
		case ERROR:
			return "ERROR";
		case WARNING:
			return "WARNING";
		case INFO:
			return "INFO";
		case DEBUG:
			return "DEBUG";
	}
	return "";
}


std::ostringstream& Output::get(Level l)
{
	level = l;
	return os;
}
