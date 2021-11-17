#pragma once
#include "ConfigServer.hpp"
#include <vector>
#include <string>
#include <map>

class Config
{
	public:
		Config(std::string const & config_file);
		~Config();
		// getter
		typedef std::vector<ConfigServer>::const_iterator	const_iterator;

		typedef std::vector<ServerBlock*> 					server_block_vector;
		typedef std::pair<std::string, int>					ip_host_pair;
		typedef std::map<ip_host_pair, server_block_vector>	address_map;
		typedef address_map::const_iterator					const_iterator_map;



		const_iterator begin() const;
		const_iterator end() const;

		// parsing
		int parser();
	
	private:
		std::string const & tokenPreper(std::string str);
		int	splitToken(std::string const & string);
		void tokenizer(std::string const & body);

		int	parseConfigFile();
		int	parseServer();
		int	parseListen();
		int	parseServerName();
		int parseClientBodySize();
		int parseAllowedMethods();
		int	parseLocation();
		int	parseAutoindex();
		int	parseErrorPage();
		int	parseCgi();
		int	parseIndex();
		int	parseRoot();
		int parseReturn();
		
		int	checkExpectedSyntax(std::string str);
		int	checkExpectedSyntax(std::string str1, std::string str2);
		int	checkExpectedSyntax(std::string str1, std::string str2, std::string str3);
		void	configError(std::string str);


	// Getters
	public:
		std::map<std::pair<std::string, int>, std::vector<ServerBlock*> >	getAddressMap();
	// Utility
	void	initAddressMap();
	

	private:
		const std::string			_file_name;
		std::vector<ConfigServer>	_servers;
		std::vector<std::string>	_tokens;
		int							_server_amount;
		size_t						_token_index;
		address_map 				_address_map;

				
		
	/* Debugging */
	public:
		void print() const;
		void printAddressMap() const;

	private:
		void printNode(const_iterator_map node) const;
		void printKey(const_iterator_map node) const;
		void printIp(const_iterator_map node) const;
		void printPort(const_iterator_map node) const;
		void printServerBlock(const_iterator_map node) const;
		void printLocationBlock(LocationBlock *location) const;
};
