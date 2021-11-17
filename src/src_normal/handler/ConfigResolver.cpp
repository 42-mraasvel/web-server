#include "ConfigResolver.hpp"
#include "settings.hpp"
#include "utility/utility.hpp"
#include <iostream>
#include <unistd.h>

ConfigResolver::ConfigResolver():
result(START),
resolved_server(NULL),
resolved_location(NULL),
auto_index(false),
redirect(false)
{}

int	ConfigResolver::resolution(Request const & request)
{
	// TODO: to delete
	ConfigMap	map;
	createConfigMap(map);

	resolved_target = request.request_target;
	ServerVector	server_vector = resolveAddress(map, request.address);
	resolved_server = resolveHost(request, server_vector);
	resolved_location = resolveLocation(request.request_target, resolved_server->getLocation());
	// Debug:
	print();

	return setResult();
}

/*****************************/
/****** resolve address ******/
/*****************************/

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

/**************************/
/****** resolve host ******/
/**************************/

ConfigServer*	ConfigResolver::resolveHost(Request const & request, ServerVector const & servers)
{
	setHost(request);

	ServerVector::const_iterator it_matched;

	if (isMatchEmpty(servers, it_matched))
	{
		return *it_matched;
	}
	if (!resolved_host.empty())
	{
		if (isMatchExactName(servers, it_matched))
		{
			return *it_matched;
		}
		if (isMatchFrontWildcard(servers, it_matched))
		{
			return *it_matched;
		}
		if (isMatchBackWildcard(servers, it_matched))
		{
			return *it_matched;
		}
	}
	return resolveDefaultHost(servers);
}

void	ConfigResolver::setHost(Request const & request)
{
	if (request.header_fields.contains("host"))
	{
		resolved_host = request.header_fields.find("host")->second;
		std::size_t found = resolved_host.rfind(":");
		if (found != std::string::npos)
		{
			resolved_host.resize(found);
		}
	}
	if (resolved_host.empty())
	{
		resolved_host = DEFAULT_SERVER_NAME;
	}
}

bool	ConfigResolver::isMatchEmpty(ServerVector const & servers, ServerVector::const_iterator & it_matched)
{
	if (resolved_host.empty())
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

bool	ConfigResolver::isMatchExactName(ServerVector const & servers, ServerVector::const_iterator & it_matched)
{
	for (it_matched = servers.begin(); it_matched != servers.end(); ++it_matched)
	{
		if (isServerNameExactMatch((*it_matched)->getServerNames()))
		{
			return true;
		}
	}
	return false;
}

bool	ConfigResolver::isServerNameExactMatch(StringVector const & server_names)
{
	for (StringVector::const_iterator it = server_names.begin(); it != server_names.end(); ++it)
	{
		if (resolved_host == *it)
		{
			return true;
		}
	}
	return false;
}

bool	ConfigResolver::isMatchFrontWildcard(ServerVector const & servers, ServerVector::const_iterator & it_matched)
{
	std::string	longest_match;
	for (ServerVector::const_iterator it = servers.begin(); it != servers.end(); ++it)
	{
		if (isServerNameFrontWildcardMatch((*it)->getServerNames(), longest_match))
		{
			it_matched = it;
		}
	}
	return !longest_match.empty();
}

bool	ConfigResolver::isServerNameFrontWildcardMatch(StringVector const & server_names, std::string & longest_match)
{
	bool	is_matched = false;
	for (StringVector::const_iterator it = server_names.begin(); it != server_names.end(); ++it)
	{
		if ((*it).size() > longest_match.size() && isFrontWildCard(*it) && isHostMatchFrontWildCard(*it))
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

bool	ConfigResolver::isHostMatchFrontWildCard(std::string const & wildcard)
{
	std::string	to_find_string = wildcard.substr(1);
	std::size_t	to_find_size = to_find_string.size();
	std::size_t	size = resolved_host.size();
	if (size < to_find_size)
	{
		return false;
	}
	return resolved_host.compare(size - to_find_size, to_find_size, to_find_string) == 0;
}

bool	ConfigResolver::isMatchBackWildcard(ServerVector const & servers, ServerVector::const_iterator & it_matched)
{
	std::string	longest_match;
	for (ServerVector::const_iterator it = servers.begin(); it != servers.end(); ++it)
	{
		if (isServerNameBackWildcardMatch((*it)->getServerNames(), longest_match))
		{
			it_matched = it;
		}
	}
	return !longest_match.empty();
}

bool	ConfigResolver::isServerNameBackWildcardMatch(StringVector const & server_names, std::string & longest_match)
{
	bool	is_matched = false;
	for (StringVector::const_iterator it = server_names.begin(); it != server_names.end(); ++it)
	{
		if ((*it).size() > longest_match.size() && isBackWildCard(*it) && isHostMatchBackWildCard(*it))
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

bool	ConfigResolver::isHostMatchBackWildCard(std::string const & wildcard)
{
	std::string	to_find_string = wildcard.substr(0, wildcard.size() - 1);
	std::size_t	to_find_size = to_find_string.size();
	std::size_t	size = resolved_host.size();
	if (size < to_find_size)
	{
		return false;
	}
	return resolved_host.compare(0, to_find_size, to_find_string) == 0;
}

ConfigServer*	ConfigResolver::resolveDefaultHost(ServerVector const & servers)
{
	return *servers.begin();
}

/******************************/
/****** resolve location ******/
/******************************/

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
	return NULL;
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
				resolved_target = temp_target;
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
		auto_index = true;
		return *it_matched;
	}
	return NULL;
}

/************************/
/****** set result ******/
/************************/

int	ConfigResolver::setResult()
{
	if (!resolved_location)
	{
		result = NOT_FOUND;
		return OK;
	}
	return scanLocation();
}

int	ConfigResolver::scanLocation()
{
	setResolvedFilePath();
	if (auto_index)
	{
		return setAutoIndexPage();
	}
	else if (resolved_location->redirect)
	{
		return setRedirect();
	}
	result = LOCATION_RESOLVED;
	return OK;
}

void	ConfigResolver::setResolvedFilePath()
{
	resolved_file_path = resolved_location->getRoot() + resolved_target;
}

int	ConfigResolver::setAutoIndexPage()
{
	if (WebservUtility::list_directory(resolved_target, resolved_file_path, auto_index_page) == ERR)
	{
		return ERR;
	}
	result = AUTO_INDEX_ON;
	return OK;
}

int	ConfigResolver::setRedirect()
{
	redirect = true;
	// TODO_config: to incorporate config
	redirect_info.first = 301;
	redirect_info.second = "This is the return text from config file.";
	result = REDIRECT;
	return OK;
}

/********************************/
/****** resolve error page ******/
/********************************/

int	ConfigResolver::resolveErrorPage(int error_code, std::string & file_path)
{
	ErrorPageInfo::const_iterator it;
	for (it = resolved_server->getErrorPages().begin(); it !=  resolved_server->getErrorPages().end(); ++it)
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
	ConfigLocation*	location = resolveLocation(error_uri, resolved_server->getLocation());
	if (!location)
	{
		return ERR;
	}
	file_path = location->getRoot() + error_uri;
	return OK;
}

/*******************/
/****** debug ******/
/*******************/

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
	new_server->addErrorPage(404, "/error/404.html");
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

void	ConfigResolver::print() const
{
	std::cout << RED_BOLD << "----------------------" << RESET_COLOR << std::endl;
	std::cout << "Config result based on hard-coded config\n(only for ConfigResolver testing):" << RESET_COLOR << std::endl;

	printSolutionServer(resolved_server);
	printSolutionLocation(resolved_location);
	printAutoIndexPage();

	std::cout << RED_BOLD << "----------------------" << RESET_COLOR << std::endl << std::endl;
}

void	ConfigResolver::printSolutionServer(ConfigServer * server) const
{
	std::cout << RED_BOLD << "Resolved server is [server_name]: ";
	StringVector names = server->getServerNames();
	for (StringVector::const_iterator it = names.begin(); it != names.end(); ++it)
	{
		std::cout << *it << " ";
	}
	std::cout << RESET_COLOR << std::endl;
	std::cout << RED_BOLD << "Resolved host is: " << resolved_host << RESET_COLOR << std::endl;
}

void	ConfigResolver::createLocations(LocationVector & locations)
{
	ConfigLocation*	new_location;
	new_location = new ConfigLocation("/");
	new_location->addRoot("./page_sample");
	// new_location->addIndex("nonexistingfile");
	// new_location->addIndex("test_index/index.html");
	new_location->addIndex("index.html");
	new_location->addAllowedMethods("GET");
	new_location->addAllowedMethods("POST");
	new_location->addAllowedMethods("DELETE");
	locations.push_back(new_location);
	// new_location = new ConfigLocation("/test_index/test_index.txt");
	// new_location->addRoot("./page_sample");
	// new_location->addAllowedMethods("GET");
	// new_location->addAllowedMethods("POST");
	// new_location->addAllowedMethods("DELETE");
	// locations.push_back(new_location);
	// new_location = new ConfigLocation("/test_index/");
	// new_location->addRoot("./page_sample");
	// new_location->addIndex("index.html");
	// new_location->addAllowedMethods("GET");
	// new_location->addAllowedMethods("POST");
	// new_location->addAllowedMethods("DELETE");
	// new_location->redirect = true;
	// locations.push_back(new_location);
	// new_location = new ConfigLocation("/auto_index/");
	// new_location->addRoot("./page_sample");
	// new_location->addIndex("nonexistingfile1");
	// new_location->addIndex("nonexistingfile2");
	// new_location->auto_index_status = true;
	// new_location->addAllowedMethods("GET");
	// new_location->addAllowedMethods("POST");
	// new_location->addAllowedMethods("DELETE");
	// locations.push_back(new_location);
	// new_location = new ConfigLocation("/error/");
	// new_location->addRoot("./page_sample");
	// new_location->addAllowedMethods("GET");
	// locations.push_back(new_location);
}

void	ConfigResolver::printSolutionLocation(ConfigLocation * location) const
{
	if (location)
	{
		std::cout << RED_BOLD << "Resolved location is [path]: " << location->getPath() << std::endl;
		std::cout << RED_BOLD << "Resolved file is: " << resolved_file_path 
				 << RESET_COLOR << std::endl;
	}
	else
	{
		std::cout << RED_BOLD << "ERROR 404!" << RESET_COLOR << std::endl;
	}
}

void	ConfigResolver::printAutoIndexPage() const
{
	if (auto_index)
	{
		std::cout << RED_BOLD << "The auto index page content is:\n" << RESET_COLOR 
				<< auto_index_page << std::endl;
	}
}
