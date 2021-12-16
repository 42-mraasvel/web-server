#pragma once
#include <vector>
#include "settings.hpp"

class ConfigParser
{
	public:
		ConfigParser(std::string const &config_file);
		int	parser();


	private:
		int _fd;
		const std::string&	_file;
		std::vector<int> _ports;

		

};













