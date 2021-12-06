#include "Webserver.hpp"
#include "settings.hpp"
#include "fd/Server.hpp"
#include "color.hpp"
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <cstdlib> // REMOVE, RM

Webserver::Webserver(address_map map): _config_map(map)
{}

int Webserver::initServer(ip_host_pair iphost)
{
	SmartPointer<Server> new_server(new Server());
	if (new_server->setupServer(iphost, &_config_map) == ERR)
	{
		return ERR;
	}
	_fd_table.insertFd(SmartPointer<AFdInfo>(new_server));
	return OK;
}

/*
TODO: close FD after failure
*/
int	Webserver::init()
{
	for (Config::const_iterator_map it = _config_map.begin(); it != _config_map.end(); ++it)
	{
		if (shouldInitialize(it->first))
		{
			printf("Initiailizing: %s:%d\n", it->first.first.c_str(), it->first.second);
			if (initServer(it->first) == ERR)
			{
				return ERR;
			}
		}
	}
	return OK;
}

bool Webserver::shouldInitialize(ip_host_pair const & iphost) const
{
	if (iphost.first != "0.0.0.0")
	{
		if (_config_map.count(ip_host_pair("0.0.0.0", iphost.second)) != 0)
		{
			return false;
		}
	}
	return true;
}

bool Webserver::shouldExecuteFd(const FdTable::AFdPointer afd)
{
	return afd->getFlag() != AFdInfo::TO_ERASE;
}

bool Webserver::shouldCloseFd(short revents) const
{
	//TODO: test on mac if this is how it functions as well
	return (revents & (POLLERR | POLLNVAL)) ||
		((revents & POLLHUP) && !(revents & POLLIN));
}

void Webserver::executeFd(short revents, FdTable::AFdPointer afd)
{
	if (shouldCloseFd(revents))
	{
		printf(BLUE_BOLD "Close Event:" RESET_COLOR " %s: [%d]\n",
			afd->getName().c_str(), afd->getFd());
		afd->closeEvent(_fd_table);
		return;
	}

	if (revents & POLLIN)
	{
		printf(BLUE_BOLD "Read event:" RESET_COLOR " %s: [%d]\n",
			afd->getName().c_str(), afd->getFd());
		afd->readEvent(_fd_table);
	}

	if (revents & POLLOUT)
	{
		printf(BLUE_BOLD "Write event:" RESET_COLOR " %s: [%d]\n",
			afd->getName().c_str(), afd->getFd());
		afd->writeEvent(_fd_table);
	}
}

//TODO: evaluate 'ready'
//TODO: add Multithreading
int	Webserver::dispatchFd(int ready)
{
	std::size_t i = 0;
	for (std::size_t i = 0; i < _fd_table.size(); ++i)
	{
		if (shouldExecuteFd(_fd_table[i].second))
		{
			try
			{
				executeFd(_fd_table[i].first.revents, _fd_table[i].second);
			}
			catch (std::exception const & e)
			{
				fprintf(stderr, "%sEXCEPTION%s: [%s]\n",
					RED_BOLD, RESET_COLOR, e.what());
				_fd_table[i].second->exceptionEvent(_fd_table);
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
		if (_fd_table[i].second->getFlag() == AFdInfo::TO_ERASE)
		{
			continue;
		}
		try
		{
			_fd_table[i].second->update(_fd_table);
		}
		catch (std::exception const & e)
		{
			fprintf(stderr, "%sUPDATE EXCEPTION%s: [%s]\n",
				RED_BOLD, RESET_COLOR, e.what());
			_fd_table[i].second->exceptionEvent(_fd_table);
		}
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
		ready = poll(_fd_table.getPointer(), _fd_table.size(), POLL_TIMEOUT);
		printf("Number of connections: %lu\n", _fd_table.size());
		if (ready < 0)
		{
			perror("Poll");
			// TODO possible exit
		}
		else if (ready > 0)
		{
			printf(YELLOW_BOLD "Poll returns: " RESET_COLOR "%d\n", ready);
			// print();
			dispatchFd(ready);
		}
		else
		{
			#ifdef __APPLE__
			#ifdef LEAK_CHECK
			//RM, REMOVE
			system("leaks debug.out");
			#endif /* LEAK_CHECK */
			#endif /* __APPLE__ */
		}
	}

	return OK;
}

/* Debugging */

void Webserver::print() const
{
	_fd_table.print();
}
