#include "ConfigResolver.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include "parser/Request.hpp"
#include "handler/CgiHandler.hpp"
#include <iostream>
#include <unistd.h>

/*********************/
/****** Getters ******/
/*********************/

ConfigInfo const & ConfigResolver::getConfigInfo() const
{
	return info;
}

/****************************/
/****** Main Interface ******/
/****************************/

ConfigResolver::ConfigResolver(MapType const * config_map)
: _config_map(config_map), _auto_index_on(false) {}

ConfigResolver::ConfigResolver(ConfigInfo::server_block_pointer server): _auto_index_on(false)
{
	info.resolved_server = server;
}

void	ConfigResolver::resolution(Request const & request, std::string const & request_host)
{
	ServerVector	server_vector = resolveAddress(request.interface_addr, *_config_map);
	info.resolved_server = resolveHost(request_host, server_vector);
	info.resolved_location = resolveLocationResult(request.method, request.request_target, info.resolved_server->_locations);
}

/*****************************/
/****** resolve address ******/
/*****************************/

ConfigResolver::ServerVector	ConfigResolver::resolveAddress(AddressType interface_address, MapType const & map)
{
	AddressType address;
	setAddress(interface_address, address, map);
	return map.find(address)->second;
}

void	ConfigResolver::setAddress(AddressType const & interface_address, AddressType & address, MapType const & map)
{
	if (map.count(interface_address) == 1)
	{
		address = interface_address;
	}
	else
	{
		address.first = "0.0.0.0";
		address.second = interface_address.second;
	}
}

/**************************/
/****** resolve host ******/
/**************************/

ConfigInfo::server_block_pointer	ConfigResolver::resolveHost(std::string const & host, ServerVector const & servers)
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

ConfigInfo::server_block_pointer	ConfigResolver::resolveDefaultHost(ServerVector const & servers)
{
	return *servers.begin();
}

/******************************/
/****** resolve location ******/
/******************************/

ConfigInfo::location_block_pointer	ConfigResolver::resolveLocationResult(Method::Type const & method, std::string const & target, LocationVectorType const & locations)
{
	info.resolved_target = target;
	_auto_index_on = false;
	ConfigInfo::location_block_pointer	location = resolveLocation(target, locations);
	info.result = getResult(location);
	if (info.result == ConfigInfo::AUTO_INDEX_ON && method != Method::GET)
	{
		location = NULL;
		info.result = ConfigInfo::NOT_FOUND;
	}
	if (info.result != ConfigInfo::NOT_FOUND)
	{
		info.resolved_file_path = location->_root + info.resolved_target;
	}
	return location;
}

ConfigInfo::location_block_pointer	ConfigResolver::resolveLocation(std::string const & target, LocationVectorType const & locations)
{
	LocationVectorType::const_iterator it_matched;

	if (!target.empty() && isMatchLocation(target, locations, it_matched))
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
		if ((*it)->_location_flag == EQUAL)
		{
			if (isExactMatch(target, path))
			{
				it_matched = it;
				return true;
			}
		}
		else
		{
			if (isPrefixMatch(target, path) && path.size() > longest_match.size())
			{
				longest_match = (*it)->_path;
				it_matched = it;
			}
		}
	}
	return !longest_match.empty();
}

bool	ConfigResolver::isExactMatch(std::string const & target, std::string const & location) const
{
	return target == location;
}

bool	ConfigResolver::isPrefixMatch(std::string const & target, std::string const & location) const
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

ConfigInfo::location_block_pointer	ConfigResolver::resolveIndex(LocationVectorType::const_iterator it_matched, std::string const & target, LocationVectorType const & locations)
{
	ConfigInfo::location_block_pointer	final_location = resolveIndexFile((*it_matched)->_index, target, locations);
	if (final_location)
	{
		return final_location;
	}
	return resolveAutoIndex(it_matched);
}

ConfigInfo::location_block_pointer	ConfigResolver::resolveIndexFile(StringVectorType indexes, std::string const & target, LocationVectorType const & locations)
{
	for (StringVectorType::const_iterator it = indexes.begin(); it != indexes.end(); ++it)
	{
		std::string temp_target = target + *it;
		ConfigInfo::location_block_pointer	final_location = resolveLocation(temp_target, locations);
		if (final_location)
		{
			std::string file = final_location->_root + temp_target;
			if (access(file.c_str(), F_OK) == OK)
			{
				info.resolved_target = temp_target;
				return final_location;
			}
			file.erase(); //TODO: to check is this necessary?
		}
		temp_target.erase(); //TODO: to check is this necessary?
	}
	return NULL;
}

ConfigInfo::location_block_pointer	ConfigResolver::resolveAutoIndex(LocationVectorType::const_iterator it_matched)
{
	if ((*it_matched)->_autoindex_status)
	{
		_auto_index_on = true;
		return *it_matched;
	}
	return NULL;
}

/************************/
/****** set result ******/
/************************/

ConfigInfo::ConfigResult	ConfigResolver::getResult(ConfigInfo::location_block_pointer location)
{
	if (!location)
	{
		return ConfigInfo::NOT_FOUND;
	}
	if (isReturnOn(location)) 
	{
		return ConfigInfo::REDIRECT;
	}
	if (isAutoIndexOn(location))
	{
		return ConfigInfo::AUTO_INDEX_ON;
	}
	return ConfigInfo::LOCATION_RESOLVED;
}

bool	ConfigResolver::isReturnOn(ConfigInfo::location_block_pointer location) const
{
	// TODO: check with Niels how to best check if no return is input in config file
	return location->_return.first != 0;
}

bool	ConfigResolver::isAutoIndexOn(ConfigInfo::location_block_pointer location) const
{
	return _auto_index_on;
}

/********************************/
/****** resolve error page ******/
/********************************/

int	ConfigResolver::resolveErrorPage(int error_code)
{
	ErrorPageType::const_iterator it;
	for (it = info.resolved_server->_error_pages.begin(); it !=  info.resolved_server->_error_pages.end(); ++it)
	{
		if (it->first == error_code)
		{
			return findErrorFilePath(it->second);
		}
	}
	return ERR;
}

//TODO: check error page in config text if it can only be uri
int	ConfigResolver::findErrorFilePath(std::string const & error_uri)
{
	ConfigInfo::location_block_pointer	location = resolveLocationResult(Method::GET, error_uri, info.resolved_server->_locations);
	if (info.result != ConfigInfo::LOCATION_RESOLVED)
	{
		return ERR;
	}
	return OK;
}

/*******************/
/****** debug ******/
/*******************/

void	ConfigResolver::print() const
{
	std::cout << RED_BOLD << "----------------------" << RESET_COLOR << std::endl;
	std::cout << "Config result based on hard-coded config\n(only for ConfigResolver testing):" << RESET_COLOR << std::endl;

	printSolutionServer(info.resolved_server);
	printSolutionLocation(info.resolved_location);

	std::cout << RED_BOLD << "----------------------" << RESET_COLOR << std::endl << std::endl;
}

void	ConfigResolver::printSolutionServer(ConfigInfo::server_block_pointer server) const
{
	std::cout << RED_BOLD << "Resolved server is [server_name]: ";
	StringVectorType names = server->_server_names;
	for (StringVectorType::const_iterator it = names.begin(); it != names.end(); ++it)
	{
		std::cout << *it << " ";
	}
	std::cout << RESET_COLOR << std::endl;
}

void	ConfigResolver::printSolutionLocation(ConfigInfo::location_block_pointer location) const
{
	if (location)
	{
		std::cout << RED_BOLD << "Resolved location is [path]: " << location->_path << std::endl;
		std::cout << RED_BOLD << "Resolved target is: " << info.resolved_target << std::endl
							  << "Resolved file is: " << info.resolved_file_path 
				  << RESET_COLOR << std::endl;
	}
	else
	{
		std::cout << RED_BOLD << "Location block failed to resolve!" << RESET_COLOR << std::endl;
	}
}
