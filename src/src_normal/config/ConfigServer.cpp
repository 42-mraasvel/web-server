#include "ConfigServer.hpp"
#include "settings.hpp"
#include <iomanip>
// TODO: Remove port 80 somewhere when process is done
ConfigServer::ConfigServer(): _client_body_size(ULONG_MAX)
{}

void	ConfigServer::addServerName(std::string name)
{
	this->_server_name.push_back(name);
}

void	ConfigServer::addClientBodySize(size_t client_body_size)
{
	this->_client_body_size = client_body_size;
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

void	ConfigServer::addLocation(ConfigLocation location)
{
	this->_locations.push_back(location);
}

void	ConfigServer::addCgi(std::string extention, std::string path)
{
	_locations[_locations.size() - 1].addCgi(extention, path);
}

void	ConfigServer::addRoot(std::string root)
{
	_locations[_locations.size() - 1].addRoot(root);
}

void	ConfigServer::addIndex(std::string index)
{
	_locations[_locations.size() - 1].addIndex(index);
}

void	ConfigServer::addAddress(std::string host, int port)
{
	_address.push_back(std::make_pair(host, port));
}


int	ConfigServer::emptyAddress()
{
	return _address.empty();
}

// Getters
// std::string ConfigServer::getHostName()
// {
// 	return this->_host;
// }

// std::vector<int> ConfigServer::getPorts()
// {
// 	return _ports;
// }

ConfigServer::const_iterator ConfigServer::begin() const
{
	return (this->_address.begin());
}

ConfigServer::const_iterator ConfigServer::end() const
{
	return (this->_address.end());
}

/* Debugging */
void ConfigServer::print() const
{
	printPorts();
	printServerName();
	printErrorPages();
	printClientBodySize();
	for (size_t i = 0; i < _locations.size(); i++)
	{	
		std::cout << YELLOW_BOLD "    Locations" RESET_COLOR " #" << (i + 1) << std::endl;
		_locations[i].print();
	}
}

void ConfigServer::printPorts() const
{
	std::cout << "  " CYAN_BOLD << "Ip:Ports:" RESET_COLOR " [";
	for (size_t i = 0; i < _address.size(); i++)
	{
		if (i > 0)
		{
			std::cout << ", ";
		}
		std::cout << _address[i].first << ":" << _address[i].second;
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

void ConfigServer::printClientBodySize() const
{
	std::cout << "  " CYAN_BOLD << "Client Body Size:" RESET_COLOR " [";
	std::cout << _client_body_size;
	std::cout << ']' << std::endl;
}
