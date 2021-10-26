#pragma once
#include "ConfigServer.hpp"
#include <vector>

#ifdef __linux__
#include <string>
#endif /* __linux__ */

class Config
{
	public:
		Config(std::string const & config_file);
		// getter
		typedef std::vector<ConfigServer>::const_iterator const_iterator;

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
		int	checkServerName(std::string name);
		int parseClientBodySize();
		int parseAllowedMethods();
		int	parseLocation();
		int	parseAutoindex();
		int	parseErrorPage();
		int	parseIndex();
		int	parseRoot();
		
		int	checkExpectedSyntax(std::string str);
		int	checkExpectedSyntax(std::string str1, std::string str2);
		int	checkExpectedSyntax(std::string str1, std::string str2, std::string str3);
		void	configError(std::string str);

	private:
		const std::string&			_file_name;
		std::vector<ConfigServer>	_servers;
		std::vector<std::string>	_tokens;
		int							_server_amount;
		size_t						_token_index;
		
	/* Debugging */
	public:
		void print() const;
};


