#pragma once
#include <vector>
#include "ConfigServer.hpp"

class Config
{
	public:
		Config(std::string const & config_file);
		// getter
		typedef std::vector<ConfigServer>::const_iterator const_iterator;

		const_iterator begin() const;
		const_iterator end() const;

	private:
		const std::string&			_file_name;
		std::vector<ConfigServer>	_servers;
		
	/* Debugging */
	public:
		void print() const;
};


