#include "Webserver.hpp"
#include "settings.hpp"
#include "fd/Server.hpp"
#include "color.hpp"
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <cstdlib> // REMOVE, RM

Webserver::Webserver(Config::address_map map): _config_map(map)
{}

int Webserver::initServer(std::pair<std::string, int> ip_host_pair)
{
	Server *new_server = new Server();
	if (new_server->setupServer(ip_host_pair.second, &_config_map) == ERR)
	{
		delete new_server;
		return ERR;
	}
	_fd_table.insertFd(new_server);
	return OK;
}

/*
TODO: close FD after failure
*/
int	Webserver::init(Config const & config)
{
	printf("Hardcoding: 8080\n");
	Server* new_server = new Server();
	if (new_server->setupServer(8080, &_config_map) == ERR)
	{
		delete new_server;
		return ERR;
	}
	_fd_table.insertFd(new_server);
	return OK;
}

bool Webserver::shouldExecuteFd(const FdTable::pair_t& fd)
{
	return fd.second->getFlag() != AFdInfo::TO_ERASE;
}

bool Webserver::shouldCloseFd(const FdTable::pair_t & fd)
{
	if (fd.first.revents & (POLLERR | POLLNVAL))
	{
		return true;
	}
	//TODO: test on mac if this is how it functions as well
	return (fd.first.revents & (POLLERR | POLLNVAL)) ||
		((fd.first.revents & POLLHUP) && !(fd.first.revents & POLLIN));
}

//TODO: evaluate 'ready'
//TODO: add Multithreading
int	Webserver::dispatchFd(int ready)
{
	std::size_t i = 0;
	for (std::size_t i = 0; i < _fd_table.size(); ++i)
	{
		if (shouldExecuteFd(_fd_table[i]))
		{
			if (shouldCloseFd(_fd_table[i]))
			{
				printf(BLUE_BOLD "Close Event:" RESET_COLOR " %s: [%d]\n",
					_fd_table[i].second->getName().c_str(), _fd_table[i].first.fd);
				_fd_table[i].second->closeEvent(_fd_table);
				continue;
			}
			if (_fd_table[i].first.revents & POLLIN)
			{
				printf(BLUE_BOLD "Read event:" RESET_COLOR " %s: [%d]\n",
					_fd_table[i].second->getName().c_str(), _fd_table[i].first.fd);
				if (_fd_table[i].second->readEvent(_fd_table) == ERR)
					return ERR;
			}
			if (_fd_table[i].first.revents & POLLOUT)
			{
				printf(BLUE_BOLD "Write event:" RESET_COLOR " %s: [%d]\n",
					_fd_table[i].second->getName().c_str(), _fd_table[i].first.fd);
				if(_fd_table[i].second->writeEvent(_fd_table) == ERR)
					return ERR;
			}
		}
	}
	return OK;
}

//TODO: scan for Timeout
void	Webserver::scanFdTable()
{
	for (std::size_t i = 0; i < _fd_table.size(); ++i)
	{
		_fd_table[i].second->update(_fd_table);
	}

	std::size_t i = 0;
	while (i < _fd_table.size())
	{
		if (_fd_table[i].second->getFlag() == AFdInfo::TO_ERASE)
		{
			printf("Erasing Fd: %s: [%d]\n",
				_fd_table[i].second->getName().c_str(), _fd_table[i].second->getFd());
			_fd_table.eraseFd(i);
			continue;
		}
		++i;
	}
}

/*
Poll loop!
*/
int	Webserver::run()
{
	int ready;

	while(true)
	{
		scanFdTable();
		ready = poll(_fd_table.getPointer(), _fd_table.size(), TIMEOUT);
		printf("Number of connections: %lu\n", _fd_table.size());
		print();
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
