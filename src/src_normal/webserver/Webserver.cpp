#include "Webserver.hpp"
#include "settings.hpp"
#include "fd/Server.hpp"

int Webserver::initServer(ConfigServer const & conf)
{
	ConfigServer::const_iterator port_it;
	for(port_it = conf.begin(); port_it != conf.end(); ++port_it)
	{
		Server *new_server = new Server();
		if (new_server->setupServer(*port_it) == ERR)
		{
			delete new_server;
			return ERR;
		}
		_fd_table.insertFd(new_server);
	}
	return OK;
}

/*
TODO: close FD after failure
*/
int	Webserver::init(Config const & config)
{
	Config::const_iterator server_it;
	for(server_it = config.begin(); server_it != config.end(); ++server_it)
	{
		if (initServer(*server_it) == ERR)
		{
			return ERR;
		}
	}
	return OK;
}

/*
Poll loop!
*/
int	Webserver::run()
{
	return OK;
}

/* Debugging */

void Webserver::print() const
{
	_fd_table.print();
}
