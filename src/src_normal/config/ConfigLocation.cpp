#include "ConfigLocation.hpp"
#include <iostream>
#include "settings.hpp"

ConfigLocation::ConfigLocation(std::string path): _path(path), _autoindex_status(1)
{}

void ConfigLocation::addRoot(std::string root)
{
	this->_root = root;
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

/* Debugging */
void	ConfigLocation::print() const
{
	printPath();
	printRoot();
	printIndex();
	printAutoIndex();
	printAllowedMethods();
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
