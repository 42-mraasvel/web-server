#include "Config.hpp"
#include "settings.hpp"

Config::Config(std::string const & config_file): _file_name(config_file)
{
	ConfigServer dummy;
	dummy.addPort(8080);
	dummy.addPort(8081);
	// dummy.addPort(8082);
	// dummy.addPort(8083);
	// ConfigServer dummy2;
	// dummy2.addPort(18000);
	// dummy2.addPort(18001);
	// dummy2.addPort(18002);
	_servers.push_back(dummy);
	// _servers.push_back(dummy2);
}

Config::const_iterator Config::begin() const
{
	return (this->_servers.begin());
}

Config::const_iterator Config::end() const
{
	return (this->_servers.end());
}

/* Debugging */

void Config::print() const
{
	std::cout << "Configuration File: " << _file_name << std::endl;
	for (const_iterator it = begin(); it != end(); ++it)
	{
		std::cout << MAGENTA_BOLD "Server" RESET_COLOR " #" << (it - begin() + 1) << std::endl;
		it->print();
	}
}
