#include "Webserver.hpp"
#include "settings.hpp"
#include "fd/Server.hpp"
#include "color.hpp"
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <cstdlib> // REMOVE, RM

int Webserver::initServer(std::pair<std::string, int> ip_host_pair)
{
	Server *new_server = new Server();
	if (new_server->setupServer(ip_host_pair) == ERR)
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
	// Config::const_iterator server_it;
	Config::address_map address_map = config.getAddressMap();
	Config::const_iterator_map server_it;
	for (server_it = address_map.begin(); server_it != address_map.end(); ++server_it)
	{
		if (initServer(server_it->first) == ERR)
		{
			return ERR;
		}
	}
	return OK;
}

//TODO: evaluate 'ready'
//TODO: add Multithreading
int	Webserver::dispatchFd(int ready)
{
	std::size_t i = 0;
	while (i < _fd_table.size())
	{
		if (_fd_table[i].second->flag != AFdInfo::TO_ERASE)
		{

			if (_fd_table[i].first.revents & POLLHUP && !(_fd_table[i].first.revents & POLLIN))
			{
				printf(BLUE_BOLD "Close Event:" RESET_COLOR " %s: [%d]\n",
					_fd_table[i].second->getName().c_str(), _fd_table[i].first.fd);
				_fd_table[i].second->closeEvent(_fd_table);
				++i;
				continue;
			}


			if (_fd_table[i].first.revents & POLLIN)
			{
				printf(BLUE_BOLD "Read event:" RESET_COLOR " %s: [%d]\n",
					_fd_table[i].second->getName().c_str(), _fd_table[i].first.fd);
				if (_fd_table[i].second->readEvent(_fd_table) == ERR)
					return ERR;
			}
			if (_fd_table[i].second->flag != AFdInfo::TO_ERASE && _fd_table[i].first.revents & POLLOUT)
			{
				printf(BLUE_BOLD "Write event:" RESET_COLOR " %s: [%d]\n",
					_fd_table[i].second->getName().c_str(), _fd_table[i].first.fd);
				if(_fd_table[i].second->writeEvent(_fd_table) == ERR)
					return ERR;
			}

		}
		++i;
	}
	return OK;
}

//TODO: scan for Timeout
void	Webserver::scanFdTable()
{
/*
DISCUSS:

First the update function is called: setting all the TO_ERASE flags if necessary
Otherwise there could be some issues in terms of setting a FD to be deleted at a previous index
The TO_ERASE function shouldn't be called inside of the update(), since it will modify the
structure and ordering of the FdTable, causing the loop invariant to be violated
*/
	for (std::size_t i = 0; i < _fd_table.size(); ++i)
	{
		//TODO: remove this if condition after the _fd_table.eraseFd() call inside update() is removed
		if (_fd_table[i].second->flag != AFdInfo::TO_ERASE) {
			_fd_table[i].second->update(_fd_table);
		}
	}

	std::size_t i = 0;
	while (i < _fd_table.size())
	{
		if (_fd_table[i].second->flag == AFdInfo::TO_ERASE)
		{
			printf("Erasing Fd: %s: [%d]\n",
				_fd_table[i].second->getName().c_str(), _fd_table[i].second->getFd());
			_fd_table.eraseFd(i);
			// i shouldn't be incremented because there is either nothing or a new FD at the same index
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
		// print();
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
