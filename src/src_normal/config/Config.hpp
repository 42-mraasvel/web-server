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
		int	parseServer(int index);
		int	parseListen(int index);
		int parseServerName(int index);
		int parseClientBodySize(int index);
		int parseAllowedMethods(int index);
		int	parseLocation(int index);
		int	parseAutoindex(int index);

	private:
		const std::string&			_file_name;
		std::vector<ConfigServer>	_servers;
		std::vector<std::string>	_tokens;
		int							_server_amount;
		
	/* Debugging */
	public:
		void print() const;
};


