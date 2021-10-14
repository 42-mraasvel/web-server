#include "ConfigServer.hpp"
#include "settings.hpp"
// TODO: Remove port 80 somewhere when process is done
ConfigServer::ConfigServer()
{
	_ports.push_back(80);
	_client_body_size= 1000000000;
	_server_name = "";
	_autoindex_status = 1;
}

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

/* Debugging */

void ConfigServer::print() const
{
	std::cout << "  " CYAN_BOLD << "Ports:" RESET_COLOR " [";
	for (const_iterator it = begin(); it != end(); ++it)
	{
		if (it != begin())
		{
			std::cout << ", ";
		}
		std::cout << *it;
	}
	std::cout << ']' << std::endl;
}
