#include "ConfigServer.hpp"
#include "settings.hpp"
#include <iostream>
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


void	ConfigServer::addPort(int port)
{
	_ports.push_back(port);
}

int	ConfigServer::emptyAddress()
{
	return _ports.empty();
}

// Getters
std::vector<int> ConfigServer::getPorts()
{
	return _ports;

}

ConfigServer::const_iterator ConfigServer::begin() const
{
	return (this->_ports.begin());
}

ConfigServer::const_iterator ConfigServer::end() const
{
	return (this->_ports.end());
}


std::vector<ServerBlock> ConfigServer::getServerBlock()
{
	initServerBlock();
	return this->_server_block;
}

std::map<std::pair<std::string, int>, std::vector<ServerBlock> > ConfigServer::getAddressMap()
{
	initAddressMap();

	return this->_address_map;
}


// Utility
void	ConfigServer::initAddressMap()
{
	initServerBlock();
	for (size_t i = 0; i < _address.size(); i++)
	{
		printAddress(i);
		// printServerBlock();
		_address_map.insert(std::make_pair(_address[i], _server_block));
	}
}


void	ConfigServer::initServerBlock()
{
	ServerBlock tmp;
	tmp._client_body_size = _client_body_size;
	tmp._server_names = _server_name;
	tmp._error_pages = _error_pages;
	for (size_t i = 0; i < _locations.size(); i++)
	{
		tmp._locations.push_back(_locations[i].getLocationBlock());
	}
	_server_block.push_back(tmp);
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

void	ConfigServer::printAddress(int index) const
{
	std::cout << BLUE_BOLD << _address[index].first;
	std::cout << ", ";
	std::cout << BLUE_BOLD << _address[index].second;
	std::cout << std::endl;
}
