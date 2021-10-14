#pragma once
#include <vector>
#include <map>
#include <string>

// class LocationConfig;

class ConfigServer
{
	public:
		ConfigServer();
		void	addPort(int port);
		typedef std::vector<int>::const_iterator const_iterator;
		const_iterator begin() const;
		const_iterator end() const;
		// getter


	private:
		std::vector<int> _ports;
		std::map<int, std::string const &> _error_pages;
		size_t _client_body_size;
		std::string _server_name;
		std::vector<std::string> _allowed_methods;
		int	_autoindex_status;

		// std::vector<LocationConfig> _locations;
		// more information added later

	/* Debugging */
	public:
		void print() const;
};
