#pragma once
#include <vector>

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
		// more information added later

};
