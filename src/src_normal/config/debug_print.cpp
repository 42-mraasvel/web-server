#include "Config.hpp"
#include "settings.hpp"
#include "utility/Output.hpp"
#include <iostream>


/* Debugging */
void Config::print() const
{
	for (const_iterator it = begin(); it != end(); ++it)
	{
		PRINT_DEBUG << MAGENTA_BOLD "Server" RESET_COLOR " #" << (it - begin() + 1) << std::endl;
		it->print();
	}
}


void Config::printAddressMap() const
{
	PRINT_DEBUG << MAGENTA_BOLD "Address Map" RESET_COLOR << std::endl;
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
	PRINT_DEBUG << YELLOW_BOLD "  Ip: " RESET_COLOR << node->first.first << std::endl;
}

void	Config::printPort(const_iterator_map node) const
{
	PRINT_DEBUG << YELLOW_BOLD "  Port: " RESET_COLOR << node->first.second << std::endl;
	
}

void	Config::printServerBlock(const_iterator_map node) const
{
	for (size_t i = 0; i < node->second.size(); i++)
	{
		PRINT_DEBUG << MAGENTA_BOLD "    ServerBlock #"  << i+1 << RESET_COLOR<< std::endl;
		PRINT_DEBUG << "\tClient size : " << node->second[i]->_client_body_size << std::endl;
		PRINT_DEBUG << "\tserver Names:" << std::endl;
		for (size_t j = 0; j < node->second[i]->_server_names.size(); j++)
		{
			PRINT_DEBUG << "\t  " << node->second[i]->_server_names[j] << std::endl;
		}
		PRINT_DEBUG << "\terror pages:" << std::endl;
		for (size_t j = 0; j < node->second[i]->_error_pages.size(); j++)
		{
			PRINT_DEBUG << "\t  " << node->second[i]->_error_pages[j].first << ", " << node->second[i]->_error_pages[j].second << std::endl;
		}
		for (size_t j = 0; j < node->second[i]->_locations.size(); j++)
		{
			PRINT_DEBUG << YELLOW_BOLD "    location #" << j+1 << RESET_COLOR << std::endl;
			printLocationBlock(node->second[i]->_locations[j]);
		}
	}
}

void	Config::printLocationBlock(ConfigLocation::location_pointer location) const
{
	PRINT_DEBUG << "\t  path: " << location->_path << std::endl;
	PRINT_DEBUG << "\t  root: " << location->_root << std::endl;
	PRINT_DEBUG << "\t  index: " ;
	for (size_t i = 0; i < location->_index.size(); i++)
	{
		if (i != 0)
		{
			PRINT_DEBUG << ", ";
		}
		PRINT_DEBUG << location->_index[i];
	}
	PRINT_DEBUG << std::endl;
	PRINT_DEBUG << "\t  Allowed Methods: " ;
	for (size_t i = 0; i < location->_allowed_methods.size(); i++)
	{
		if (i != 0)
		{
			PRINT_DEBUG << ", ";
		}
		PRINT_DEBUG << location->_allowed_methods[i];
	}
	PRINT_DEBUG << std::endl;
	PRINT_DEBUG << "\t  CGI: " ;
	for (size_t i = 0; i < location->_cgi.size(); i++)
	{
		if (i != 0)
		{
			PRINT_DEBUG <<"; ";
		}
		PRINT_DEBUG << location->_cgi[i].first << ", " << location->_cgi[i].second;
	}
	PRINT_DEBUG << std::endl;
	PRINT_DEBUG << "\t  Autoindex status: ";
	if (location->_autoindex_status)
	{
		PRINT_DEBUG << "ON";
	}
	else
	{
		PRINT_DEBUG << "OFF";
	}
	PRINT_DEBUG << std::endl;
	PRINT_DEBUG << "\t  Return: " ;
	PRINT_DEBUG << location->_return.first << ", " << location->_return.second;
	PRINT_DEBUG << std::endl;
	PRINT_DEBUG << "\t  Location flag: ";
	if (location->_location_flag == NONE)
	{
		PRINT_DEBUG << "NONE";
	}
	else if (location->_location_flag == EQUAL)
	{
		PRINT_DEBUG << "EQUAL";
	}
	PRINT_DEBUG << std::endl;
	PRINT_DEBUG << "\t  Upload_store: ";
	PRINT_DEBUG << location->_upload_store << std::endl;
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
		PRINT_DEBUG << YELLOW_BOLD "    Locations" RESET_COLOR " #" << (i + 1) << std::endl;
		_locations[i].print();
	}
}

void ConfigServer::printPorts() const
{
	PRINT_DEBUG << "  " CYAN_BOLD << "Ports:" RESET_COLOR " [";
	for (const_iterator it = begin(); it != end(); ++it)
	{
		if (it != begin())
		{
			PRINT_DEBUG << ", ";
		}
		PRINT_DEBUG << *it;
	}
	PRINT_DEBUG << ']' << std::endl;
}

void ConfigServer::printServerName() const
{
	PRINT_DEBUG << "  " CYAN_BOLD << "Server names:" RESET_COLOR " [";
	for (size_t i = 0; i < _server_name.size(); i++)
	{
		if (i != 0)
		{
			PRINT_DEBUG << ", ";
		}
		PRINT_DEBUG << _server_name[i];
	}
	PRINT_DEBUG << ']' << std::endl;
}

void ConfigServer::printErrorPages() const
{
		PRINT_DEBUG << "  " CYAN_BOLD << "Error_pages:" RESET_COLOR " [";
	for (size_t i = 0; i < _error_pages.size(); i++)
	{
		if (i != 0)
		{
			PRINT_DEBUG << ", ";
		}
		PRINT_DEBUG << " { "<< _error_pages[i].first << ", " << _error_pages[i].second << " } ";
	}
	PRINT_DEBUG << ']' << std::endl;
}

void	ConfigServer::printAddress() const
{
	PRINT_DEBUG << "  " CYAN_BOLD << "Addresses:" RESET_COLOR " [";
	for (size_t i = 0; i < _address.size(); i++)
	{
		if (i != 0)
		{
			PRINT_DEBUG << ", ";
		}
		PRINT_DEBUG << " { "<< _address[i].first << ", " << _address[i].second << " } ";
	}
	PRINT_DEBUG << ']' << std::endl;
}


void ConfigServer::printClientBodySize() const
{
	PRINT_DEBUG << "  " CYAN_BOLD << "Client Body Size:" RESET_COLOR " [";
	PRINT_DEBUG << _client_body_size;
	PRINT_DEBUG << ']' << std::endl;
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
	PRINT_DEBUG << "  \t" WHITE_BOLD << "Path:" RESET_COLOR " [";
	if (!_path.empty())
	{
		PRINT_DEBUG << _path;
	}
	PRINT_DEBUG << ']' << std::endl;
}

void	ConfigLocation::printRoot() const
{
	PRINT_DEBUG << "  \t" WHITE_BOLD << "Root:" RESET_COLOR " [";
	if (!_root.empty())
	{	
		PRINT_DEBUG << _root;
	}
	PRINT_DEBUG << ']' << std::endl;
}

void ConfigLocation::printAllowedMethods() const
{
		PRINT_DEBUG << "  \t" WHITE_BOLD << "Allowed Methods:" RESET_COLOR " [";
	for (size_t i = 0; i < _allowed_methods.size(); i++)
	{
		if (i != 0)
		{
			PRINT_DEBUG << ", ";
		}
		PRINT_DEBUG << _allowed_methods[i];
	}
	PRINT_DEBUG << ']' << std::endl;
}

void ConfigLocation::printAutoIndex() const
{
	PRINT_DEBUG << "  \t" WHITE_BOLD << "Autoindex:" RESET_COLOR " [";
	if (_autoindex_status == 1)
	{
		PRINT_DEBUG << "ON";
	}
	else
	{
		PRINT_DEBUG << "OFF";
	}
	PRINT_DEBUG << ']' << std::endl;
}

void ConfigLocation::printIndex() const
{
		PRINT_DEBUG << "  \t" WHITE_BOLD << "Index:" RESET_COLOR " [";
	for (size_t i = 0; i < _index.size(); i++)
	{
		if (i != 0)
		{
			PRINT_DEBUG << ", ";
		}
		PRINT_DEBUG << _index[i];
	}
	PRINT_DEBUG << ']' << std::endl;
}

void ConfigLocation::printCgi() const
{
	PRINT_DEBUG << "  \t" WHITE_BOLD << "CGI:" RESET_COLOR " [";
	for (size_t i = 0; i < _cgi.size(); i++)
	{
		if (i != 0)
		{
			PRINT_DEBUG << ", ";
		}
		PRINT_DEBUG << _cgi[i].first << ";" << _cgi[i].second;
	}
	PRINT_DEBUG << ']' << std::endl;
}

void	ConfigLocation::printUpload() const
{
	PRINT_DEBUG << "  \t" WHITE_BOLD << "Upload_store:" RESET_COLOR " [";
	if (!_upload_store.empty())
	{	
		PRINT_DEBUG << _upload_store;
	}
	PRINT_DEBUG << ']' << std::endl;
}
