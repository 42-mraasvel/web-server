#include "ConfigServer.hpp"

ConfigServer::ConfigServer() {}

void	ConfigServer::addPort(int port)
{
	this->_ports.push_back(port);
}

ConfigServer::const_iterator ConfigServer::begin() const
{
	return (this->_ports.begin());
}

ConfigServer::const_iterator ConfigServer::end() const
{
	return (this->_ports.end());
}
