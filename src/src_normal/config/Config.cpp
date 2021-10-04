#include "Config.hpp"

Config::Config(std::string const & config_file): _file_name(config_file)
{
	ConfigServer dummy;
	dummy.addPort(8080);
	this->_servers.push_back(dummy);
}

Config::const_iterator Config::begin() const
{
	return (this->_servers.begin());
}

Config::const_iterator Config::end() const
{
	return (this->_servers.end());
}
