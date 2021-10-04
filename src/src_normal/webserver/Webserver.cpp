#include "Webserver.hpp"
#include "settings.hpp"
#include "fd/Server.hpp"



int	Webserver::runWebserver()
{
	return OK;
}

int	Webserver::initWebserver(Config const & config)
{
	Config::const_iterator configIterator;
	ConfigServer::const_iterator portIterator;

	for(configIterator = config.begin();
		configIterator != config.end();
		++configIterator)
	{
		for(portIterator = configIterator->begin();
			portIterator != configIterator->end();
			++portIterator)
		{
			Server *tmp = new Server();
			tmp->initServer(*portIterator);
			this->_fd_table.insertFd(tmp);
		}
	}
	return OK;
}






