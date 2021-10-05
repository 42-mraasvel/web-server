#include "Webserver.hpp"
#include "settings.hpp"
#include "fd/Server.hpp"
#include "color.hpp"
#include <sys/socket.h>
#include <poll.h>

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

//TODO: evaluate 'ready'
int	Webserver::dispatchFd(int ready)
{
	std::size_t i = 0;
	while (i < _fd_table.size())
	{
		if (_fd_table[i].first.revents & POLLHUP)
		{
			printf(BLUE_BOLD "Close Event:" RESET_COLOR " [%d]\n", _fd_table[i].first.fd);
			_fd_table.eraseFd(_fd_table[i].second->getIndex());
			continue;
		}
		if (_fd_table[i].first.revents & POLLIN)
		{
			printf(BLUE_BOLD "Read event:" RESET_COLOR " [%d]\n", _fd_table[i].first.fd);
			_fd_table[i].second->readEvent(_fd_table);
		}
		if (_fd_table[i].first.revents & POLLOUT)
		{
			printf(BLUE_BOLD "Write event:" RESET_COLOR " [%d]\n", _fd_table[i].first.fd);
			_fd_table[i].second->writeEvent(_fd_table);
		}
		++i;
	}
	return OK;
}

/*
Poll loop!
*/
int	Webserver::run()
{
	int ready;

	while(true)
	{
		//TODO: scan table for FDs that should be removed: TIMEOUT or FILES
		ready = poll(_fd_table.getPointer(), _fd_table.size(), TIMEOUT);
		printf("Number of connections: %lu\n", _fd_table.size());
		if (ready < 0)
		{
			perror("Poll");
			// TODO possible exit
		}	
		else if (ready > 0)
		{
			printf(YELLOW_BOLD "Poll returns: " RESET_COLOR "%d\n", ready);
			dispatchFd(ready);
		}
	}

	return OK;
}

/* Debugging */

void Webserver::print() const
{
	_fd_table.print();
}
