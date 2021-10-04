#include "Webserver.hpp"
#include "settings.hpp"
#include "fd/Server.hpp"

int	Webserver::runWebserver()
{
	return OK;
}

int	Webserver::initWebserver(Config const & config)
{

	Config::const_iterator server_it;
	for(server_it = config.begin(); server_it != config.end(); ++server_it)
	{
		ConfigServer::const_iterator port_it;
		for(port_it = server_it->begin(); port_it != server_it->end(); ++port_it)
		{
			Server *new_server = new Server();
			if (new_server->initServer(*port_it) == ERR)
			{
				delete new_server;
				return ERR;
			}
			this->_fd_table.insertFd(new_server);
		}
	}
	return OK;
}
