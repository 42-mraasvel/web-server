#pragma once
#include <vector>
#include <map>
#include <string>
#include "ConfigLocation.hpp"

typedef struct s_ServerBlock
{
	std::vector<std::pair<std::string, int> >	_address;
	size_t 						_client_body_size;
	std::vector<std::string>	_server_names;
	std::vector<std::pair<int, std::string> >	_error_pages;
	std::vector<ConfigLocation::location_pointer> _locations;
}				ServerBlock;


class ConfigServer
{
	public:
		typedef SmartPointer<ServerBlock> server_pointer;
		typedef std::vector<server_pointer> 				server_block_vector;
		typedef std::pair<std::string, int>					ip_host_pair;
		typedef std::map<ip_host_pair, server_block_vector>	address_map;

		ConfigServer();
		void	addPort(int port);
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
		void	addReturn(int code, std::string path);
		void	addLocationFlag(location_flag flag);
		int		emptyAddress();

	// getters
		std::string getHostName();
		std::vector<int> getPorts();
		address_map getAddressMap();

	// Utility 
		void	initServerBlock();
		void	initAddressMap();


		typedef std::vector<int>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;

	private:
		int											_default_port;
		size_t										_client_body_size;
		std::vector<int>							_ports;
		std::vector<std::pair<std::string, int> >	_address;
		std::vector<std::string>					_server_name;
		std::vector<std::pair<int, std::string> >	_error_pages;
		std::vector<ConfigLocation>					_locations;
		server_block_vector							_server_block;
		address_map									_address_map;


		// more information added later

	/* Debugging */
	public:
		void print() const;
	
	private:
		void printPorts() const;
		void printServerName() const;
		void printClientBodySize() const;
		void printErrorPages() const;
		void printAddress() const;
};
