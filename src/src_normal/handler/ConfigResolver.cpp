#include "ConfigResolver.hpp"
#include "settings.hpp"
#include <iostream>
#include <unistd.h>

ConfigResolver::ConfigResolver(std::string const & request_target): _auto_index(false), _new_target(request_target) {}

void	ConfigResolver::resolution(Request const & request)
{
	// TODO: to delete
	ConfigMap	map;
	createConfigMap(map);

	ServerVector	server_vector = resolveAddress(map, request.address);
	ConfigServer*	server = resolveHost(request, server_vector);
	ConfigLocation*	location = resolveLocation(request.request_target, server->getLocation());
	
	printSolutionServer(server);
	printSolutionLocation(location);
}

ConfigResolver::ServerVector	ConfigResolver::resolveAddress(ConfigMap map, Request::Address client_address)
{
	Request::Address	address;
	setAddress(map, client_address, address);
	return map.find(address)->second;
}

void	ConfigResolver::setAddress(ConfigMap const & map, Request::Address const & client_address, Request::Address & address)
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

ConfigServer*	ConfigResolver::resolveHost(Request const & request, ServerVector const & servers)
{
	std::string	host;
	setHost(request, host);

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

void	ConfigResolver::setHost(Request const & request, std::string & host)
{
	if (request.header_fields.contains("host"))
	{
		host = request.header_fields.find("host")->second;
		std::size_t found = host.rfind(":");
		if (found != std::string::npos)
		{
			host.resize(found);
		}
	}
}

bool	ConfigResolver::isMatchEmpty(std::string const & host, ServerVector const & servers, ServerVector::const_iterator & it_matched)
{
	if (host.empty())
	{
		for (it_matched = servers.begin(); it_matched != servers.end(); ++it_matched)
		{
			if (isServerNameEmpty((*it_matched)->getServerNames()))
			{
				return true;
			}
		}
	}
	return false;
}

// TODO: check how to save "": empty string? or '\0'?
bool	ConfigResolver::isServerNameEmpty(StringVector const & server_names)
{
	if (server_names.empty())
	{
		return true;
	}
	for (StringVector::const_iterator it = server_names.begin(); it != server_names.end(); ++it)
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
		if (isServerNameExactMatch(host, (*it_matched)->getServerNames()))
		{
			return true;
		}
	}
	return false;
}

bool	ConfigResolver::isServerNameExactMatch(std::string const & host, StringVector const & server_names)
{
	for (StringVector::const_iterator it = server_names.begin(); it != server_names.end(); ++it)
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
		if (isServerNameFrontWildcardMatch(host, (*it)->getServerNames(), longest_match))
		{
			it_matched = it;
		}
	}
	return !longest_match.empty();
}

bool	ConfigResolver::isServerNameFrontWildcardMatch(std::string const & host, StringVector const & server_names, std::string & longest_match)
{
	bool	is_matched = false;
	for (StringVector::const_iterator it = server_names.begin(); it != server_names.end(); ++it)
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
		if (isServerNameBackWildcardMatch(host, (*it)->getServerNames(), longest_match))
		{
			it_matched = it;
		}
	}
	return !longest_match.empty();
}

bool	ConfigResolver::isServerNameBackWildcardMatch(std::string const & host, StringVector const & server_names, std::string & longest_match)
{
	bool	is_matched = false;
	for (StringVector::const_iterator it = server_names.begin(); it != server_names.end(); ++it)
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

ConfigServer*	ConfigResolver::resolveDefaultHost(ServerVector const & servers)
{
	/* TODO: to confirm with team 'default' flag is not needed in config file. is so, delete below for loop.
	for (ServerVector::const_iterator it = servers.begin(); it != servers.end(); ++it)
	{
		if (*it is the default server)
		{
			return *it;
		}
	}
	*/
	return *servers.begin();
}

ConfigLocation*	ConfigResolver::resolveLocation(std::string const & request_target, LocationVector const & locations)
{
	LocationVector::const_iterator it_matched;

	if (isMatchLocation(request_target, locations, it_matched))
	{
		if (isTargetDirectory(request_target))
		{
			return resolveIndex(it_matched, request_target, locations);
		}
		else
		{
			return *it_matched;
		}
	}
	return NULL; // TODO: return 404 not found
}

bool	ConfigResolver::isMatchLocation(std::string const & request_target, LocationVector const & locations, LocationVector::const_iterator & it_matched)
{
	std::string longest_match;

	for (LocationVector::const_iterator it = locations.begin(); it != locations.end(); ++it)
	{
		std::string path = (*it)->getPath();
		if (isPrefixMatch(request_target, path) && path.size() > longest_match.size())
		{
			longest_match = (*it)->getPath();
			it_matched = it;
		}
	}
	return !longest_match.empty();
}

bool	ConfigResolver::isPrefixMatch(std::string const & request_target, std::string const & location)
{
	if (request_target.size() >= location.size())
	{
		return request_target.compare(0, location.size(), location) == 0;
	}
	return false;
}

bool	ConfigResolver::isTargetDirectory(std::string const & target)
{
	return target[target.size() - 1] == '/';
}

ConfigLocation*	ConfigResolver::resolveIndex(LocationVector::const_iterator it_matched, std::string const & request_target, LocationVector const & locations)
{
	ConfigLocation*	final_location = resolveIndexFile((*it_matched)->getIndex(), request_target, locations);
	if (final_location)
	{
		return final_location;
	}
	return resolveAutoIndex(it_matched);
}

ConfigLocation*	ConfigResolver::resolveIndexFile(StringVector indexes, std::string const & request_target, LocationVector const & locations)
{
	for (StringVector::const_iterator it = indexes.begin(); it != indexes.end(); ++it)
	{
		std::string temp_target = request_target + *it;
		ConfigLocation*	final_location = resolveLocation(temp_target, locations);
		if (final_location)
		{
			std::string file = final_location->getRoot() + temp_target;
			if (access(file.c_str(), F_OK) == OK)
			{
				_new_target = temp_target;
				return final_location;
			}
		}
	}
	return NULL;
}

ConfigLocation*	ConfigResolver::resolveAutoIndex(LocationVector::const_iterator it_matched)
{
	if ((*it_matched)->auto_index_status)
	{
		_auto_index = true;
		return *it_matched;
	}
	return NULL;
}

//TODO: to delete
void	ConfigResolver::createConfigMap(ConfigMap & map)
{
	LocationVector locations;
	createLocations(locations);

	ServerVector servers;
	createServers(servers, locations);

	std::pair< std::string, int > address("127.0.0.1", 8080);
	map[address] = servers;
}

void	ConfigResolver::createServers(ServerVector & servers, LocationVector const & locations)
{
	ConfigServer*	new_server;
	new_server = new ConfigServer;
	new_server->addServerName("localhost");
	new_server->addServerName("localhost.com");
	new_server->_locationptrs = locations;
	servers.push_back(new_server);
	new_server = new ConfigServer;
	new_server->addServerName("*.com");
	new_server->_locationptrs = locations;
	servers.push_back(new_server);
	new_server = new ConfigServer;
	new_server->addServerName("*.com.cn");
	new_server->_locationptrs = locations;
	servers.push_back(new_server);
	new_server = new ConfigServer;
	new_server->addServerName("*.cn");
	new_server->_locationptrs = locations;
	servers.push_back(new_server);
	new_server = new ConfigServer;
	new_server->addServerName("www.*");
	new_server->_locationptrs = locations;
	servers.push_back(new_server);
	new_server = new ConfigServer;
	new_server->addServerName("www.xxx.*");
	new_server->addServerName("");
	new_server->_locationptrs = locations;
	servers.push_back(new_server);
	new_server = new ConfigServer;
	new_server->_locationptrs = locations;
	servers.push_back(new_server);
}

void	ConfigResolver::printSolutionServer(ConfigServer * server)
{
	std::cout << RED_BOLD << "Resolved server is [server_name]: ";
	StringVector names = server->getServerNames();
	for (StringVector::const_iterator it = names.begin(); it != names.end(); ++it)
	{
		std::cout << *it << " ";
	}
	std::cout << RESET_COLOR << std::endl;
}

void	ConfigResolver::createLocations(LocationVector & locations)
{
	ConfigLocation*	new_location;
	new_location = new ConfigLocation("/");
	new_location->addIndex("nonexistingfile");
	new_location->addIndex("test_index/index.html");
	new_location->addIndex("index.html");
	new_location->addRoot("./page_sample");
	locations.push_back(new_location);
	new_location = new ConfigLocation("/test_index/test_index.txt");
	new_location->addRoot("./page_sample");
	locations.push_back(new_location);
	new_location = new ConfigLocation("/test_index/");
	new_location->addIndex("index.html");
	new_location->addRoot("./page_sample");
	locations.push_back(new_location);
	new_location = new ConfigLocation("/autoindex/");
	new_location->addIndex("nonexistingfile1");
	new_location->addIndex("nonexistingfile2");
	new_location->auto_index_status = true;
	new_location->addRoot("./page_sample");
	locations.push_back(new_location);
}

void	ConfigResolver::printSolutionLocation(ConfigLocation * location)
{
	if (location)
	{
		_resolved_file_path = location->getRoot() + _new_target;
		std::cout << RED_BOLD << "Resolved location is [path]: " << location->getPath() << std::endl;
		std::cout << RED_BOLD << "Resolved file is: " << _resolved_file_path 
				 << RESET_COLOR << std::endl;
	}
	else
	{
		std::cout << RED_BOLD << "ERROR 404!" << RESET_COLOR << std::endl;
	}
}
