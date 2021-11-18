#include "ConfigResolver.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "parser/Request.hpp"
#include <iostream>
#include <unistd.h>

ConfigResolver::ConfigResolver():
resolved_server(NULL),
resolved_location(NULL)
result(START),
{}

void	ConfigResolver::resolution(MapType const & map, AddressType const & request_address, std::string const & request_host, std::string const & request_target)
{
	ServerVector	server_vector = resolveAddress(request_address, map);
	resolved_server = resolveHost(request_host, server_vector);
	resolved_location = resolveLocation(request_target, resolved_server->_locations);
	result = getResult(resolved_location);
	if (result != NOT_FOUND)
	{
		resolved_file_path = getResolvedFilePath();
	}
}

/*****************************/
/****** resolve address ******/
/*****************************/

ConfigResolver::ServerVector	ConfigResolver::resolveAddress(AddressType client_address, MapType const & map)
{
	AddressType address;
	setAddress(client_address, address, map);
	return map.find(address)->second;
}

void	ConfigResolver::setAddress(AddressType const & client_address, AddressType & address, MapType const & map)
{
	if (map.count(client_address) == 1)
	{
		address = client_address;
	}
	else
	{
		address.first = "0.0.0.0";
		address.second = client_address.second;
	}
}

/**************************/
/****** resolve host ******/
/**************************/

ServerBlock*	ConfigResolver::resolveHost(std::string const & host, ServerVector const & servers)
{
	ServerVector::const_iterator it_matched;

	if (isMatchEmpty(host, servers, it_matched))
	{
		return *it_matched;
	}
	if (!host.empty())
	{
		if (isMatchExactName(host, servers, it_matched))
		{
			return *it_matched;
		}
		if (isMatchFrontWildcard(host, servers, it_matched))
		{
			return *it_matched;
		}
		if (isMatchBackWildcard(host, servers, it_matched))
		{
			return *it_matched;
		}
	}
	return resolveDefaultHost(servers);
}

bool	ConfigResolver::isMatchEmpty(std::string const & host, ServerVector const & servers, ServerVector::const_iterator & it_matched)
{
	if (host.empty())
	{
		for (it_matched = servers.begin(); it_matched != servers.end(); ++it_matched)
		{
			if (isServerNameEmpty((*it_matched)->_server_names))
			{
				return true;
			}
		}
	}
	return false;
}

bool	ConfigResolver::isServerNameEmpty(StringVectorType const & server_names)
{
	if (server_names.empty())
	{
		return true;
	}
	for (StringVectorType::const_iterator it = server_names.begin(); it != server_names.end(); ++it)
	{
		if ((*it).empty())
		{
			return true;
		}
	}
	return false;
}

bool	ConfigResolver::isMatchExactName(std::string const & host, ServerVector const & servers, ServerVector::const_iterator & it_matched)
{
	for (it_matched = servers.begin(); it_matched != servers.end(); ++it_matched)
	{
		if (isServerNameExactMatch(host, (*it_matched)->_server_names))
		{
			return true;
		}
	}
	return false;
}

bool	ConfigResolver::isServerNameExactMatch(std::string const & host, StringVectorType const & server_names)
{
	for (StringVectorType::const_iterator it = server_names.begin(); it != server_names.end(); ++it)
	{
		if (host == *it)
		{
			return true;
		}
	}
	return false;
}

bool	ConfigResolver::isMatchFrontWildcard(std::string const & host, ServerVector const & servers, ServerVector::const_iterator & it_matched)
{
	std::string	longest_match;
	for (ServerVector::const_iterator it = servers.begin(); it != servers.end(); ++it)
	{
		if (isServerNameFrontWildcardMatch(host, (*it)->_server_names, longest_match))
		{
			it_matched = it;
		}
	}
	return !longest_match.empty();
}

bool	ConfigResolver::isServerNameFrontWildcardMatch(std::string const & host, StringVectorType const & server_names, std::string & longest_match)
{
	bool	is_matched = false;
	for (StringVectorType::const_iterator it = server_names.begin(); it != server_names.end(); ++it)
	{
		if ((*it).size() > longest_match.size() && isFrontWildCard(*it) && isHostMatchFrontWildCard(host, *it))
		{
			longest_match = *it;
			is_matched = true;
		}
	}
	return is_matched;
}

bool	ConfigResolver::isFrontWildCard(std::string const & string)
{
	return string.size() >= 2 && string[0] == '*';
}

bool	ConfigResolver::isHostMatchFrontWildCard(std::string const & host, std::string const & wildcard)
{
	std::string	to_find_string = wildcard.substr(1);
	std::size_t	to_find_size = to_find_string.size();
	std::size_t	size = host.size();
	if (size < to_find_size)
	{
		return false;
	}
	return host.compare(size - to_find_size, to_find_size, to_find_string) == 0;
}

bool	ConfigResolver::isMatchBackWildcard(std::string const & host, ServerVector const & servers, ServerVector::const_iterator & it_matched)
{
	std::string	longest_match;
	for (ServerVector::const_iterator it = servers.begin(); it != servers.end(); ++it)
	{
		if (isServerNameBackWildcardMatch(host, (*it)->_server_names, longest_match))
		{
			it_matched = it;
		}
	}
	return !longest_match.empty();
}

bool	ConfigResolver::isServerNameBackWildcardMatch(std::string const & host, StringVectorType const & server_names, std::string & longest_match)
{
	bool	is_matched = false;
	for (StringVectorType::const_iterator it = server_names.begin(); it != server_names.end(); ++it)
	{
		if ((*it).size() > longest_match.size() && isBackWildCard(*it) && isHostMatchBackWildCard(host, *it))
		{
			longest_match = *it;
			is_matched = true;
		}
	}
	return is_matched;
}

bool	ConfigResolver::isBackWildCard(std::string const & string)
{
	return string.size() >= 2 && string[string.size() - 1] == '*';
}

bool	ConfigResolver::isHostMatchBackWildCard(std::string const & host, std::string const & wildcard)
{
	std::string	to_find_string = wildcard.substr(0, wildcard.size() - 1);
	std::size_t	to_find_size = to_find_string.size();
	std::size_t	size = host.size();
	if (size < to_find_size)
	{
		return false;
	}
	return host.compare(0, to_find_size, to_find_string) == 0;
}

ServerBlock*	ConfigResolver::resolveDefaultHost(ServerVector const & servers)
{
	return *servers.begin();
}

/******************************/
/****** resolve location ******/
/******************************/

LocationBlock*	ConfigResolver::resolveLocation(std::string const & target, LocationVectorType const & locations)
{
	LocationVectorType::const_iterator it_matched;
	resolved_target = target;

	if (isMatchLocation(target, locations, it_matched))
	{
		if (isTargetDirectory(target))
		{
			return resolveIndex(it_matched, target, locations);
		}
		else
		{
			return *it_matched;
		}
	}
	return NULL;
}

bool	ConfigResolver::isMatchLocation(std::string const & target, LocationVectorType const & locations, LocationVectorType::const_iterator & it_matched)
{
	std::string longest_match;

	for (LocationVectorType::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		std::string path = (*it)->_path;
		if (isPrefixMatch(target, path) && path.size() > longest_match.size())
		{
			longest_match = (*it)->_path;
			it_matched = it;
		}
	}
	return !longest_match.empty();
}

bool	ConfigResolver::isPrefixMatch(std::string const & target, std::string const & location)
{
	if (target.size() >= location.size())
	{
		return target.compare(0, location.size(), location) == 0;
	}
	return false;
}

bool	ConfigResolver::isTargetDirectory(std::string const & target)
{
	return target[target.size() - 1] == '/';
}

LocationBlock*	ConfigResolver::resolveIndex(LocationVectorType::const_iterator it_matched, std::string const & target, LocationVectorType const & locations)
{
	LocationBlock*	final_location = resolveIndexFile((*it_matched)->_index, target, locations);
	if (final_location)
	{
		return final_location;
	}
	return resolveAutoIndex(it_matched);
}

LocationBlock*	ConfigResolver::resolveIndexFile(StringVectorType indexes, std::string const & target, LocationVectorType const & locations)
{
	for (StringVectorType::const_iterator it = indexes.begin(); it != indexes.end(); ++it)
	{
		std::string temp_target = target + *it;
		LocationBlock*	final_location = resolveLocation(temp_target, locations);
		if (final_location)
		{
			std::string file = final_location->_root + temp_target;
			if (access(file.c_str(), F_OK) == OK)
			{
				resolved_target = temp_target;
				return final_location;
			}
			file.erase(); //TODO: to check is this necessary?
		}
		temp_target.erase(); //TODO: to check is this necessary?
	}
	return NULL;
}

LocationBlock*	ConfigResolver::resolveAutoIndex(LocationVectorType::const_iterator it_matched)
{
	if ((*it_matched)->_autoindex_status)
	{
		return *it_matched;
	}
	return NULL;
}

/************************/
/****** set result ******/
/************************/

ConfigResolver::ConfigResult	ConfigResolver::getResult(LocationBlock* location)
{
	if (!location)
	{
		return NOT_FOUND;
	}
	if (isReturnOn(location)) 
	{
		return REDIRECT;
	}
	else if (isAutoIndexOn(location))
	{
		return AUTO_INDEX_ON;
	}
	return LOCATION_RESOLVED;
}

bool	ConfigResolver::isReturnOn(LocationBlock* location) const
{
	// TODO: check with Niels how to best check if no return is input in config file
	return location->_return.first != 0;
}

bool	ConfigResolver::isAutoIndexOn(LocationBlock* location) const
{
	return resolved_location->_autoindex_status;
}

std::string	ConfigResolver::getResolvedFilePath()
{
	return resolved_location->_root + resolved_target;
}


/*
int	ConfigResolver::setAutoIndexPage()
{
	//To move to set message body
//	if (WebservUtility::list_directory(resolved_target, resolved_file_path, auto_index_page) == ERR)
//	{
//		return ERR;
//	}
	result = AUTO_INDEX_ON;
	return OK;
}
*/

/********************************/
/****** resolve error page ******/
/********************************/

int	ConfigResolver::resolveErrorPage(int error_code, std::string & file_path)
{
	ErrorPageType::const_iterator it;
	for (it = resolved_server->_error_pages.begin(); it !=  resolved_server->_error_pages.end(); ++it)
	{
		if (it->first == error_code)
		{
			return findErrorFilePath(it->second, file_path);
		}
	}
	return ERR;
}

int	ConfigResolver::findErrorFilePath(std::string const & error_uri, std::string & file_path)
{
	LocationBlock*	location = resolveLocation(error_uri, resolved_server->_locations);
	ConfigResult 	result = getResult(location);
	if (result != LOCATION_RESOLVED)
	{
		return ERR;
	}
	file_path = getResolvedFilePath();
	return OK;
}

/*******************/
/****** debug ******/
/*******************/

void	ConfigResolver::print() const
{
	std::cout << RED_BOLD << "----------------------" << RESET_COLOR << std::endl;
	std::cout << "Config result based on hard-coded config\n(only for ConfigResolver testing):" << RESET_COLOR << std::endl;

	printSolutionServer(resolved_server);
	printSolutionLocation(resolved_location);

	std::cout << RED_BOLD << "----------------------" << RESET_COLOR << std::endl << std::endl;
}

void	ConfigResolver::printSolutionServer(ServerBlock * server) const
{
	std::cout << RED_BOLD << "Resolved server is [server_name]: ";
	StringVectorType names = server->_server_names;
	for (StringVectorType::const_iterator it = names.begin(); it != names.end(); ++it)
	{
		std::cout << *it << " ";
	}
	std::cout << RESET_COLOR << std::endl;
}

void	ConfigResolver::printSolutionLocation(LocationBlock * location) const
{
	if (location)
	{
		std::cout << RED_BOLD << "Resolved location is [path]: " << location->_path << std::endl;
		std::cout << RED_BOLD << "Resolved file is: " << resolved_file_path 
				 << RESET_COLOR << std::endl;
	}
	else
	{
		std::cout << RED_BOLD << "ERROR 404!" << RESET_COLOR << std::endl;
	}
}
