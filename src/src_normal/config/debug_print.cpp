#include "Config.hpp"
#include "settings.hpp"
#include "outputstream/Output.hpp"
#include <iostream>


/* Debugging */
void Config::print() const
{
	for (const_iterator it = begin(); it != end(); ++it)
	{
		PRINT << MAGENTA_BOLD "Server" RESET_COLOR " #" << (it - begin() + 1) << std::endl;
		it->print();
	}
}


void Config::printAddressMap() const
{
	PRINT << MAGENTA_BOLD "Address Map" RESET_COLOR << std::endl;
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
	PRINT << YELLOW_BOLD "  Ip: " RESET_COLOR << node->first.first << std::endl;
}

void	Config::printPort(const_iterator_map node) const
{
	PRINT << YELLOW_BOLD "  Port: " RESET_COLOR << node->first.second << std::endl;
	
}

void	Config::printServerBlock(const_iterator_map node) const
{
	for (size_t i = 0; i < node->second.size(); i++)
	{
		PRINT << MAGENTA_BOLD "    ServerBlock #"  << i+1 << RESET_COLOR<< std::endl;
		PRINT << "\tClient size : " << node->second[i]->_client_body_size << std::endl;
		PRINT << "\tserver Names:" << std::endl;
		for (size_t j = 0; j < node->second[i]->_server_names.size(); j++)
		{
			PRINT << "\t  " << node->second[i]->_server_names[j] << std::endl;
		}
		PRINT << "\terror pages:" << std::endl;
		for (size_t j = 0; j < node->second[i]->_error_pages.size(); j++)
		{
			PRINT << "\t  " << node->second[i]->_error_pages[j].first << ", " << node->second[i]->_error_pages[j].second << std::endl;
		}
		for (size_t j = 0; j < node->second[i]->_locations.size(); j++)
		{
			PRINT << YELLOW_BOLD "    location #" << j+1 << RESET_COLOR << std::endl;
			printLocationBlock(node->second[i]->_locations[j]);
		}
	}
}

void	Config::printLocationBlock(ConfigLocation::location_pointer location) const
{
	PRINT << "\t  path: " << location->_path << std::endl;
	PRINT << "\t  root: " << location->_root << std::endl;
	PRINT << "\t  index: " ;
	for (size_t i = 0; i < location->_index.size(); i++)
	{
		if (i != 0)
		{
			PRINT << ", ";
		}
		PRINT << location->_index[i];
	}
	PRINT << std::endl;
	PRINT << "\t  Allowed Methods: " ;
	for (size_t i = 0; i < location->_allowed_methods.size(); i++)
	{
		if (i != 0)
		{
			PRINT << ", ";
		}
		PRINT << location->_allowed_methods[i];
	}
	PRINT << std::endl;
	PRINT << "\t  CGI: " ;
	for (size_t i = 0; i < location->_cgi.size(); i++)
	{
		if (i != 0)
		{
			PRINT <<"; ";
		}
		PRINT << location->_cgi[i].first << ", " << location->_cgi[i].second;
	}
	PRINT << std::endl;
	PRINT << "\t  Autoindex status: ";
	if (location->_autoindex_status)
	{
		PRINT << "ON";
	}
	else
	{
		PRINT << "OFF";
	}
	PRINT << std::endl;
	PRINT << "\t  Return: " ;
	PRINT << location->_return.first << ", " << location->_return.second;
	PRINT << std::endl;
	PRINT << "\t  Location flag: ";
	if (location->_location_flag == NONE)
	{
		PRINT << "NONE";
	}
	else if (location->_location_flag == EQUAL)
	{
		PRINT << "EQUAL";
	}
	PRINT << std::endl;
	PRINT << "\t  Upload_store: ";
	PRINT << location->_upload_store << std::endl;
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
		PRINT << YELLOW_BOLD "    Locations" RESET_COLOR " #" << (i + 1) << std::endl;
		_locations[i].print();
	}
}

void ConfigServer::printPorts() const
{
	PRINT << "  " CYAN_BOLD << "Ports:" RESET_COLOR " [";
	for (const_iterator it = begin(); it != end(); ++it)
	{
		if (it != begin())
		{
			PRINT << ", ";
		}
		PRINT << *it;
	}
	PRINT << ']' << std::endl;
}

void ConfigServer::printServerName() const
{
	PRINT << "  " CYAN_BOLD << "Server names:" RESET_COLOR " [";
	for (size_t i = 0; i < _server_name.size(); i++)
	{
		if (i != 0)
		{
			PRINT << ", ";
		}
		PRINT << _server_name[i];
	}
	PRINT << ']' << std::endl;
}

void ConfigServer::printErrorPages() const
{
		PRINT << "  " CYAN_BOLD << "Error_pages:" RESET_COLOR " [";
	for (size_t i = 0; i < _error_pages.size(); i++)
	{
		if (i != 0)
		{
			PRINT << ", ";
		}
		PRINT << " { "<< _error_pages[i].first << ", " << _error_pages[i].second << " } ";
	}
	PRINT << ']' << std::endl;
}

void	ConfigServer::printAddress() const
{
	PRINT << "  " CYAN_BOLD << "Addresses:" RESET_COLOR " [";
	for (size_t i = 0; i < _address.size(); i++)
	{
		if (i != 0)
		{
			PRINT << ", ";
		}
		PRINT << " { "<< _address[i].first << ", " << _address[i].second << " } ";
	}
	PRINT << ']' << std::endl;
}


void ConfigServer::printClientBodySize() const
{
	PRINT << "  " CYAN_BOLD << "Client Body Size:" RESET_COLOR " [";
	PRINT << _client_body_size;
	PRINT << ']' << std::endl;
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
	PRINT << "  \t" WHITE_BOLD << "Path:" RESET_COLOR " [";
	if (!_path.empty())
	{
		PRINT << _path;
	}
	PRINT << ']' << std::endl;
}

void	ConfigLocation::printRoot() const
{
	PRINT << "  \t" WHITE_BOLD << "Root:" RESET_COLOR " [";
	if (!_root.empty())
	{	
		PRINT << _root;
	}
	PRINT << ']' << std::endl;
}

void ConfigLocation::printAllowedMethods() const
{
		PRINT << "  \t" WHITE_BOLD << "Allowed Methods:" RESET_COLOR " [";
	for (size_t i = 0; i < _allowed_methods.size(); i++)
	{
		if (i != 0)
		{
			PRINT << ", ";
		}
		PRINT << _allowed_methods[i];
	}
	PRINT << ']' << std::endl;
}

void ConfigLocation::printAutoIndex() const
{
	PRINT << "  \t" WHITE_BOLD << "Autoindex:" RESET_COLOR " [";
	if (_autoindex_status == 1)
	{
		PRINT << "ON";
	}
	else
	{
		PRINT << "OFF";
	}
	PRINT << ']' << std::endl;
}

void ConfigLocation::printIndex() const
{
		PRINT << "  \t" WHITE_BOLD << "Index:" RESET_COLOR " [";
	for (size_t i = 0; i < _index.size(); i++)
	{
		if (i != 0)
		{
			PRINT << ", ";
		}
		PRINT << _index[i];
	}
	PRINT << ']' << std::endl;
}

void ConfigLocation::printCgi() const
{
	PRINT << "  \t" WHITE_BOLD << "CGI:" RESET_COLOR " [";
	for (size_t i = 0; i < _cgi.size(); i++)
	{
		if (i != 0)
		{
			PRINT << ", ";
		}
		PRINT << _cgi[i].first << ";" << _cgi[i].second;
	}
	PRINT << ']' << std::endl;
}

void	ConfigLocation::printUpload() const
{
	PRINT << "  \t" WHITE_BOLD << "Upload_store:" RESET_COLOR " [";
	if (!_upload_store.empty())
	{	
		PRINT << _upload_store;
	}
	PRINT << ']' << std::endl;
}
