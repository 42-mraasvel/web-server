#include "ConfigResolver.hpp"
#include "settings.hpp"
#include <iostream>

void	ConfigResolver::resolution(Request const & request)
{
	ConfigMap	map; //TODO

	ServerVector	server_vector = resolveAddress(map, request.address);
	ConfigServer*	server = resolveHost(request, server_vector);
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
	bool		default_host = !request.header_fields.contains("host");;
	std::string	host;
	if (!default_host)
	{
		setHost(request, host);
	}
	for (ServerVector::const_iterator it = servers.begin(); it != servers.end(); ++it)
	{
		if (isHostMatched(host, (*it)->getServerNames()))
		{
			return *it;
		}
	}
	return *servers.begin();
}

void	ConfigResolver::setHost(Request const & request, std::string & host)
{
	host = request.header_fields.find("host")->second;
	std::size_t found = host.rfind(":");
	if (found != std::string::npos)
	{
		host.resize(found);
	}
}

bool	ConfigResolver::isHostMatched(std::string const & host, StringVector const & server_names)
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
