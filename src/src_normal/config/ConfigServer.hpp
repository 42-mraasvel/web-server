#pragma once
#include <vector>
#include <map>
#include <string>
#include "ConfigLocation.hpp"

class ConfigServer
{
	public:
		ConfigServer();
		void	addServerName(std::string name);
		void	addClientBodySize(size_t client_body_size);
		void	addAllowedMethods(std::string method);
		void	addAutoIndex(int status);
		void	addIndex(std::string index);
		void	addErrorPage(int page_number, std::string path);
		void	addLocation(ConfigLocation location);
		void	addRoot(std::string root);
		void	addCgi(std::string extention, std::string path);
		void	addAddress(std::string host, int port);

		int		emptyAddress();
		// getters
		std::string getHostName();
		std::vector<int> getPorts();




		typedef std::vector<std::pair<std::string, int> >::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

	private:
		int											_default_port;
		size_t										_client_body_size;
		std::vector<std::pair<std::string, int> >	_address;
		std::vector<std::string>					_server_name;
		std::vector<std::pair<int, std::string> >	_error_pages;
		std::vector<ConfigLocation>					_locations;

		// more information added later

	/* Debugging */
	public:
		void print() const;
	
	// private:
		void printPorts() const;
		void printServerName() const;
		void printClientBodySize() const;
		void printErrorPages() const;
};
