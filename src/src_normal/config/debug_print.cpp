#include "Config.hpp"
#include "settings.hpp"
#include <iostream>


/* Debugging */
void Config::print() const
{
	for (const_iterator it = begin(); it != end(); ++it)
	{
		std::cout << MAGENTA_BOLD "Server" RESET_COLOR " #" << (it - begin() + 1) << std::endl;
		it->print();
	}
}


void Config::printAddressMap() const
{
	std::cout << MAGENTA_BOLD "Address Map" RESET_COLOR << std::endl;
	for (const_iterator_map it = _address_map.begin(); it != _address_map.end(); ++it)
	{
		printNode(it);
	}
}

void Config::printNode(const_iterator_map node) const
{
	printKey(node);
	printServerBlock(node);
}

void	Config::printKey(const_iterator_map node) const
{
	printIp(node);
	printPort(node);
}

void	Config::printIp(const_iterator_map node) const
{
	std::cout << YELLOW_BOLD "  Ip: " RESET_COLOR << node->first.first << std::endl;
}

void	Config::printPort(const_iterator_map node) const
{
	std::cout << YELLOW_BOLD "  Port: " RESET_COLOR << node->first.second << std::endl;
	
}

void	Config::printServerBlock(const_iterator_map node) const
{
	for (size_t i = 0; i < node->second.size(); i++)
	{
		std::cout << MAGENTA_BOLD "    ServerBlock #"  << i+1 << RESET_COLOR<< std::endl;
		std::cout << "\tClient size : " << node->second[i]->_client_body_size << std::endl;
		std::cout << "\tserver Names:" << std::endl;
		for (size_t j = 0; j < node->second[i]->_server_names.size(); j++)
		{
			std::cout << "\t  " << node->second[i]->_server_names[j] << std::endl;
		}
		std::cout << "\terror pages:" << std::endl;
		for (size_t j = 0; j < node->second[i]->_error_pages.size(); j++)
		{
			std::cout << "\t  " << node->second[i]->_error_pages[j].first << ", " << node->second[i]->_error_pages[j].second << std::endl;
		}
		for (size_t j = 0; j < node->second[i]->_locations.size(); j++)
		{
			std::cout << YELLOW_BOLD "    location #" << j+1 << RESET_COLOR << std::endl;
			printLocationBlock(node->second[i]->_locations[j]);
		}
	}
}

void	Config::printLocationBlock(ConfigLocation::location_pointer location) const
{
	std::cout << "\t  path: " << location->_path << std::endl;
	std::cout << "\t  root: " << location->_root << std::endl;
	std::cout << "\t  index: " ;
	for (size_t i = 0; i < location->_index.size(); i++)
	{
		if (i != 0)
		{
			std::cout << ", ";
		}
		std::cout << location->_index[i];
	}
	std::cout << std::endl;
	std::cout << "\t  Allowed Methods: " ;
	for (size_t i = 0; i < location->_allowed_methods.size(); i++)
	{
		if (i != 0)
		{
			std::cout << ", ";
		}
		std::cout << location->_allowed_methods[i];
	}
	std::cout << std::endl;
	std::cout << "\t  CGI: " ;
	for (size_t i = 0; i < location->_cgi.size(); i++)
	{
		if (i != 0)
		{
			std::cout <<"; ";
		}
		std::cout << location->_cgi[i].first << ", " << location->_cgi[i].second;
	}
	std::cout << std::endl;
	std::cout << "\t  Autoindex status: ";
	if (location->_autoindex_status)
	{
		std::cout << "ON";
	}
	else
	{
		std::cout << "OFF";
	}
	std::cout << std::endl;
	std::cout << "\t  Return: " ;
	std::cout << location->_return.first << ", " << location->_return.second;
	std::cout << std::endl;
	std::cout << "\t  Location flag: ";
	if (location->_location_flag == NONE)
	{
		std::cout << "NONE";
	}
	else if (location->_location_flag == EQUAL)
	{
		std::cout << "EQUAL";
	}
	std::cout << std::endl;
	std::cout << "\t  Upload_store: ";
	std::cout << location->_upload_store << std::endl;
}


/* Debugging */
void ConfigServer::print() const
{
	printAddress();
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

void	ConfigServer::printAddress() const
{
	std::cout << "  " CYAN_BOLD << "addresses:" RESET_COLOR " [";
	for (size_t i = 0; i < _address.size(); i++)
	{
		if (i != 0)
		{
			std::cout << ", ";
		}
		std::cout << " { "<< _address[i].first << ", " << _address[i].second << " } ";
	}
	std::cout << ']' << std::endl;
}


void ConfigServer::printClientBodySize() const
{
	std::cout << "  " CYAN_BOLD << "Client Body Size:" RESET_COLOR " [";
	std::cout << _client_body_size;
	std::cout << ']' << std::endl;
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
	printUpload();
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

void	ConfigLocation::printUpload() const
{
	std::cout << "  \t" WHITE_BOLD << "Upload_store:" RESET_COLOR " [";
	if (!_upload_store.empty())
	{	
		std::cout << _upload_store;
	}
	std::cout << ']' << std::endl;
}



