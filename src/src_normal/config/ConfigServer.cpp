#include "ConfigServer.hpp"
#include "settings.hpp"
#include <iomanip>
// TODO: Remove port 80 somewhere when process is done
ConfigServer::ConfigServer(){}

void	ConfigServer::addPort(int port)
{
	this->_ports.push_back(port);
}

void	ConfigServer::addServerName(std::string name)
{
	this->_server_name.push_back(name);
}

void	ConfigServer::addClientBodySize(size_t client_body_size)
{
	this->_locations[_locations.size() - 1].addClientBodySize(client_body_size);
}

void	ConfigServer::addAllowedMethods(std::string method)
{
	this->_locations[_locations.size() - 1].addAllowedMethods(method);
}

void	ConfigServer::addAutoIndex(int status)
{
	this->_locations[_locations.size() - 1].addAutoIndex(status);
}

void	ConfigServer::addErrorPage(int page_number, std::string path)
{
	_error_pages.push_back(std::pair<int, std::string>(page_number, path));
}

void	ConfigServer::addHostName(std::string name)
{
	this->_host = name;
}

void	ConfigServer::addLocation(ConfigLocation location)
{
	this->_locations.push_back(location);
}

void	ConfigServer::addRoot(std::string root)
{
	_locations[_locations.size() - 1].addRoot(root);
}

void	ConfigServer::addIndex(std::string index)
{
	_locations[_locations.size() - 1].addIndex(index);
}

int	ConfigServer::hostIsEmpty()
{
	return _host.empty();
}

int	ConfigServer::portIsEmpty()
{
	return _ports.empty();
}

std::string ConfigServer::getHostName()
{
	return this->_host;
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
	printPorts();
	printHostName();
	printServerName();
	printErrorPages();
	for (size_t i = 0; i < _locations.size(); i++)
	{	
		std::cout << YELLOW_BOLD "    Locations" RESET_COLOR " #" << (i + 1) << std::endl;
		_locations[i].print();
	}
}

void ConfigServer::printPorts() const
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

void ConfigServer::printServerName() const
{
	std::cout << "  " CYAN_BOLD << "Server names:" RESET_COLOR " [";
	for (size_t i = 0; i < _server_name.size(); i++)
	{
		if (i != 0)
		{
			std::cout << ", ";
		}
		std::cout << _server_name[i];
	}
	std::cout << ']' << std::endl;
}

void ConfigServer::printErrorPages() const
{
		std::cout << "  " CYAN_BOLD << "Error_pages:" RESET_COLOR " [";
	for (size_t i = 0; i < _error_pages.size(); i++)
	{
		if (i != 0)
		{
			std::cout << ", ";
		}
		std::cout << " { "<< _error_pages[i].first << ", " << _error_pages[i].second << " } ";
	}
	std::cout << ']' << std::endl;
}

void ConfigServer::printHostName() const
{
		std::cout << "  " CYAN_BOLD << "Host name:" RESET_COLOR " [";
	if(!_host.empty())
	{
		std::cout << _host;
	}
	std::cout << ']' << std::endl;
}

//TODO: only for ConfigResolver, to delete
std::vector<std::string> const &	ConfigServer::getServerNames() 
{
	return _server_name;
}

std::vector<ConfigLocation * > const &	ConfigServer::getLocation() 
{
	return _locationptrs;
}
