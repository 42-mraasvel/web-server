#pragma once
#include <vector>
#include <map>
#include <string>
#include "ConfigLocation.hpp"

class ConfigServer
{
	public:
		ConfigServer();
		void	addPort(int port);
		void	addServerName(std::string name);
		void	addClientBodySize(size_t client_body_size);
		void	addAllowedMethods(std::string method);
		void	addAutoIndex(int status);
		void	addIndex(std::string index);
		void	addErrorPage(int page_number, std::string path);
		void	addHostName(std::string host);
		void	addLocation(ConfigLocation location);
		void	addRoot(std::string root);
		int		hostIsEmpty();
		int		portIsEmpty();

		std::string getHostName();

		typedef std::vector<int>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

		//TODO: only for ConfigResolver, to delete
		std::vector<std::string> const &	getServerNames();

	private:
		int											_default_port;
		std::string									_host;
		std::vector<int> 							_ports;
		std::vector<std::string>					_server_name;
		std::vector<std::pair<int, std::string> >	_error_pages;
		std::vector<ConfigLocation>					_locations;

		// more information added later

	/* Debugging */
	public:
		void print() const;
	
	private:
		void printPorts() const;
		void printServerName() const;
		void printHostName() const;
		void printClientBodySize() const;
		void printAllowedMethods() const;
		void printAutoIndex() const;
		void printErrorPages() const;
};
