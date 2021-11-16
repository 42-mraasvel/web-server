#include "ConfigLocation.hpp"
#include <iostream>
#include "settings.hpp"

ConfigLocation::ConfigLocation(std::string path): _path(path), _autoindex_status(1)
{}

void ConfigLocation::addRoot(std::string root)
{
	this->_root = root;
}

void ConfigLocation::addClientBodySize(size_t client_body_size)
{
	this->_client_body_size = client_body_size;
}

void	ConfigLocation::addAllowedMethods(std::string method)
{
	this->_allowed_methods.push_back(method);
}

void	ConfigLocation::addAutoIndex(int status)
{
	this->_autoindex_status = status;
}

void	ConfigLocation::addIndex(std::string index)
{
	this->_index.push_back(index);
}

void	ConfigLocation::addCgi(std::string extention, std::string path)
{
	std::pair<std::string, std::string> cgi(extention, path);
	this->_cgi.push_back(cgi);
}

void ConfigLocation::addReturn(int code, std::string path)
{
	this->_return = std::make_pair(code, path);
}

LocationBlock* ConfigLocation::getLocationBlock()
{
	initLocationBlock();
	return this->_location_block;
}

void ConfigLocation::initLocationBlock()
{
	_location_block = new LocationBlock;
	_location_block->_path = _path;
	_location_block->_root = _root;
	_location_block->_index = _index;
	_location_block->_allowed_methods = _allowed_methods;
	_location_block->_cgi = _cgi;
	_location_block->_autoindex_status = _autoindex_status;
	_location_block->_return = _return;
}

/* Debugging */
void	ConfigLocation::print() const
{
	printPath();
	printRoot();
	printIndex();
	printAutoIndex();
	printAllowedMethods();
	printClientBodySize();
	printCgi();
}

void	ConfigLocation::printPath() const
{
	std::cout << "  \t" WHITE_BOLD << "Path:" RESET_COLOR " [";
	if (!_path.empty())
	{
		std::cout << _path;
	}
	std::cout << ']' << std::endl;
}

void	ConfigLocation::printRoot() const
{
	std::cout << "  \t" WHITE_BOLD << "Root:" RESET_COLOR " [";
	if (!_root.empty())
	{	
		std::cout << _root;
	}
	std::cout << ']' << std::endl;
}

void ConfigLocation::printClientBodySize() const
{
	std::cout << "  \t" WHITE_BOLD << "Client Body Size:" RESET_COLOR " [";
	std::cout << _client_body_size;
	std::cout << ']' << std::endl;
}

void ConfigLocation::printAllowedMethods() const
{
		std::cout << "  \t" WHITE_BOLD << "Allowed Methods:" RESET_COLOR " [";
	for (size_t i = 0; i < _allowed_methods.size(); i++)
	{
		if (i != 0)
		{
			std::cout << ", ";
		}
		std::cout << _allowed_methods[i];
	}
	std::cout << ']' << std::endl;
}

void ConfigLocation::printAutoIndex() const
{
	std::cout << "  \t" WHITE_BOLD << "Autoindex:" RESET_COLOR " [";
	if (_autoindex_status == 1)
	{
		std::cout << "ON";
	}
	else
	{
		std::cout << "OFF";
	}
	std::cout << ']' << std::endl;
}

void ConfigLocation::printIndex() const
{
		std::cout << "  \t" WHITE_BOLD << "Index:" RESET_COLOR " [";
	for (size_t i = 0; i < _index.size(); i++)
	{
		if (i != 0)
		{
			std::cout << ", ";
		}
		std::cout << _index[i];
	}
	std::cout << ']' << std::endl;
}

void ConfigLocation::printCgi() const
{
	std::cout << "  \t" WHITE_BOLD << "CGI:" RESET_COLOR " [";
	for (size_t i = 0; i < _cgi.size(); i++)
	{
		if (i != 0)
		{
			std::cout << ", ";
		}
		std::cout << _cgi[i].first << ";" << _cgi[i].second;
	}
	std::cout << ']' << std::endl;
}




